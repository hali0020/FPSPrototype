param(
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '..\SourceAssets\Audio')
)

$sampleRate = 48000
$twoPi = 2.0 * [Math]::PI

function Write-PcmWave {
    param(
        [Parameter(Mandatory = $true)] [double[]]$Samples,
        [Parameter(Mandatory = $true)] [string]$OutputPath,
        [Parameter(Mandatory = $true)] [double]$TargetPeak,
        [double]$EndFadeSeconds = 0.035
    )

    $sampleCount = $Samples.Length
    $processed = [double[]]::new($sampleCount)
    $windows = [double[]]::new($sampleCount)
    $previousInput = 0.0
    $previousOutput = 0.0
    $dcPole = 0.993
    $startFadeSamples = [Math]::Max(1, [int](0.0010 * $sampleRate))
    $endFadeSamples = [Math]::Max(2, [int]($EndFadeSeconds * $sampleRate))

    for ($index = 0; $index -lt $sampleCount; ++$index) {
        $filtered = $Samples[$index] - $previousInput + $dcPole * $previousOutput
        $previousInput = $Samples[$index]
        $previousOutput = $filtered

        $window = 1.0
        if ($index -lt $startFadeSamples) {
            $phase = $index / [double]$startFadeSamples
            $window *= [Math]::Sin(0.5 * [Math]::PI * $phase)
        }
        if ($index -ge $sampleCount - $endFadeSamples) {
            $phase = ($sampleCount - 1 - $index) / [double]($endFadeSamples - 1)
            $window *= [Math]::Sin(0.5 * [Math]::PI * [Math]::Max(0.0, $phase))
        }

        $windows[$index] = $window
        $processed[$index] = [Math]::Tanh($filtered * 1.08) * $window
    }

    # Remove any residual numerical DC without disturbing the digital-zero edges.
    $weightedSum = 0.0
    $windowSum = 0.0
    for ($index = 0; $index -lt $sampleCount; ++$index) {
        $weightedSum += $processed[$index]
        $windowSum += $windows[$index]
    }
    $dcCorrection = if ($windowSum -gt 0.0) { $weightedSum / $windowSum } else { 0.0 }

    $peak = 0.0
    for ($index = 0; $index -lt $sampleCount; ++$index) {
        $processed[$index] -= $dcCorrection * $windows[$index]
        $peak = [Math]::Max($peak, [Math]::Abs($processed[$index]))
    }
    $normalization = if ($peak -gt 0.0) { $TargetPeak / $peak } else { 1.0 }

    $resolvedOutput = [System.IO.Path]::GetFullPath($OutputPath)
    [System.IO.Directory]::CreateDirectory(
        [System.IO.Path]::GetDirectoryName($resolvedOutput)) | Out-Null

    $stream = [System.IO.File]::Create($resolvedOutput)
    $writer = [System.IO.BinaryWriter]::new($stream)
    try {
        $dataSize = $sampleCount * 2
        $writer.Write([System.Text.Encoding]::ASCII.GetBytes('RIFF'))
        $writer.Write([int](36 + $dataSize))
        $writer.Write([System.Text.Encoding]::ASCII.GetBytes('WAVE'))
        $writer.Write([System.Text.Encoding]::ASCII.GetBytes('fmt '))
        $writer.Write([int]16)
        $writer.Write([int16]1)
        $writer.Write([int16]1)
        $writer.Write([int]$sampleRate)
        $writer.Write([int]($sampleRate * 2))
        $writer.Write([int16]2)
        $writer.Write([int16]16)
        $writer.Write([System.Text.Encoding]::ASCII.GetBytes('data'))
        $writer.Write([int]$dataSize)

        foreach ($sample in $processed) {
            $pcm = [int16][Math]::Round(
                [Math]::Clamp($sample * $normalization, -1.0, 1.0) * 32767.0)
            $writer.Write($pcm)
        }
    }
    finally {
        $writer.Dispose()
        $stream.Dispose()
    }

    $duration = $sampleCount / [double]$sampleRate
    Write-Output "Generated $resolvedOutput ($sampleRate Hz mono, 16-bit PCM, $duration s)"
}

function New-AmmoPickupSound {
    $durationSeconds = 0.30
    $sampleCount = [int]($sampleRate * $durationSeconds)
    $samples = [double[]]::new($sampleCount)
    $random = [System.Random]::new(731021)
    $lowNoise = 0.0

    for ($index = 0; $index -lt $sampleCount; ++$index) {
        $time = $index / [double]$sampleRate
        $noise = $random.NextDouble() * 2.0 - 1.0
        $lowNoise += 0.12 * ($noise - $lowNoise)
        $highNoise = $noise - $lowNoise
        $value = 0.0

        foreach ($impact in @(
            @{ Start = 0.000; Level = 1.00; Phase = 0.20 },
            @{ Start = 0.092; Level = 0.62; Phase = 1.35 }
        )) {
            $localTime = $time - $impact.Start
            if ($localTime -ge 0.0) {
                $attack = [Math]::Min(1.0, $localTime / 0.00035)
                $clickEnvelope = [Math]::Exp(-$localTime / 0.0055)
                $metalEnvelope = [Math]::Exp(-$localTime / 0.044)
                $bodyEnvelope = [Math]::Exp(-$localTime / 0.019)
                $metal = (
                    [Math]::Sin($twoPi * 2381.0 * $localTime + $impact.Phase) * 0.34 +
                    [Math]::Sin($twoPi * 3479.0 * $localTime + 0.8 + $impact.Phase) * 0.24 +
                    [Math]::Sin($twoPi * 4937.0 * $localTime + 2.1 + $impact.Phase) * 0.13
                ) * $metalEnvelope
                $body = (
                    [Math]::Sin($twoPi * 196.0 * $localTime + 0.4) * 0.19 +
                    [Math]::Sin($twoPi * 317.0 * $localTime + 1.2) * 0.12
                ) * $bodyEnvelope
                $value += ($highNoise * $clickEnvelope * 0.42 + $metal + $body) *
                    $attack * $impact.Level
            }
        }

        # Short deterministic bolt-slide texture between the two latch impacts.
        if ($time -ge 0.038 -and $time -lt 0.094) {
            $slidePhase = ($time - 0.038) / 0.056
            $slideWindow = [Math]::Pow([Math]::Sin([Math]::PI * $slidePhase), 2.0)
            $value += $highNoise * $slideWindow * 0.105
        }

        $samples[$index] = $value
    }
    return $samples
}

function New-HealthPickupSound {
    $durationSeconds = 0.42
    $sampleCount = [int]($sampleRate * $durationSeconds)
    $samples = [double[]]::new($sampleCount)
    $random = [System.Random]::new(412909)
    $airState = 0.0

    for ($index = 0; $index -lt $sampleCount; ++$index) {
        $time = $index / [double]$sampleRate
        $noise = $random.NextDouble() * 2.0 - 1.0
        $airState += 0.045 * ($noise - $airState)
        $value = 0.0

        foreach ($tone in @(
            @{ Start = 0.012; Frequency = 659.255; Level = 0.34; Decay = 0.145 },
            @{ Start = 0.092; Frequency = 830.609; Level = 0.30; Decay = 0.155 },
            @{ Start = 0.172; Frequency = 987.767; Level = 0.25; Decay = 0.165 }
        )) {
            $localTime = $time - $tone.Start
            if ($localTime -ge 0.0) {
                $attack = 1.0 - [Math]::Exp(-$localTime / 0.009)
                $envelope = [Math]::Exp(-$localTime / $tone.Decay) * $attack
                $fundamental = [Math]::Sin($twoPi * $tone.Frequency * $localTime)
                $warmPartial = [Math]::Sin(
                    $twoPi * ($tone.Frequency * 2.0) * $localTime + 0.35) * 0.14
                $value += ($fundamental + $warmPartial) * $envelope * $tone.Level
            }
        }

        $airEnvelope = (1.0 - [Math]::Exp(-$time / 0.018)) * [Math]::Exp(-$time / 0.19)
        $value += $airState * $airEnvelope * 0.055
        $samples[$index] = $value
    }
    return $samples
}

function New-SupplyPickupSound {
    $durationSeconds = 0.50
    $sampleCount = [int]($sampleRate * $durationSeconds)
    $samples = [double[]]::new($sampleCount)
    $random = [System.Random]::new(880301)
    $lowNoise = 0.0

    for ($index = 0; $index -lt $sampleCount; ++$index) {
        $time = $index / [double]$sampleRate
        $noise = $random.NextDouble() * 2.0 - 1.0
        $lowNoise += 0.10 * ($noise - $lowNoise)
        $highNoise = $noise - $lowNoise

        $attack = [Math]::Min(1.0, $time / 0.0005)
        $latchEnvelope = [Math]::Exp(-$time / 0.010)
        $ringEnvelope = [Math]::Exp(-$time / 0.067)
        $bodyEnvelope = [Math]::Exp(-$time / 0.072)
        $value = (
            $highNoise * $latchEnvelope * 0.31 +
            [Math]::Sin($twoPi * 1723.0 * $time + 0.4) * $ringEnvelope * 0.18 +
            [Math]::Sin($twoPi * 2861.0 * $time + 1.7) * $ringEnvelope * 0.12 +
            [Math]::Sin($twoPi * 130.813 * $time) * $bodyEnvelope * 0.22 +
            [Math]::Sin($twoPi * 261.626 * $time + 0.25) * $bodyEnvelope * 0.12
        ) * $attack

        foreach ($tone in @(
            @{ Start = 0.055; Frequency = 523.251; Level = 0.24 },
            @{ Start = 0.112; Frequency = 659.255; Level = 0.24 },
            @{ Start = 0.169; Frequency = 783.991; Level = 0.23 }
        )) {
            $localTime = $time - $tone.Start
            if ($localTime -ge 0.0) {
                $toneAttack = 1.0 - [Math]::Exp(-$localTime / 0.0055)
                $toneEnvelope = [Math]::Exp(-$localTime / 0.175) * $toneAttack
                $value += (
                    [Math]::Sin($twoPi * $tone.Frequency * $localTime) +
                    [Math]::Sin($twoPi * ($tone.Frequency * 2.0) * $localTime + 0.45) * 0.18 +
                    [Math]::Sin($twoPi * ($tone.Frequency * 0.5) * $localTime + 0.2) * 0.12
                ) * $toneEnvelope * $tone.Level
            }
        }

        # A second locking click makes the combined supply case feel substantial.
        $secondTime = $time - 0.205
        if ($secondTime -ge 0.0) {
            $secondAttack = [Math]::Min(1.0, $secondTime / 0.0004)
            $secondEnvelope = [Math]::Exp(-$secondTime / 0.017)
            $value += (
                $highNoise * 0.13 +
                [Math]::Sin($twoPi * 2147.0 * $secondTime + 0.7) * 0.12
            ) * $secondEnvelope * $secondAttack
        }

        $samples[$index] = $value
    }
    return $samples
}

$resolvedDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)
[System.IO.Directory]::CreateDirectory($resolvedDirectory) | Out-Null

$ammoSamples = [double[]](New-AmmoPickupSound)
$healthSamples = [double[]](New-HealthPickupSound)
$supplySamples = [double[]](New-SupplyPickupSound)

Write-PcmWave -Samples $ammoSamples `
    -OutputPath (Join-Path $resolvedDirectory 'SFX_Pickup_Ammo_01.wav') `
    -TargetPeak 0.82 -EndFadeSeconds 0.040
Write-PcmWave -Samples $healthSamples `
    -OutputPath (Join-Path $resolvedDirectory 'SFX_Pickup_Health_01.wav') `
    -TargetPeak 0.66 -EndFadeSeconds 0.065
Write-PcmWave -Samples $supplySamples `
    -OutputPath (Join-Path $resolvedDirectory 'SFX_Pickup_Supply_01.wav') `
    -TargetPeak 0.80 -EndFadeSeconds 0.070
