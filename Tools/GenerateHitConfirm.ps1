param(
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\SourceAssets\Audio\SFX_HitConfirm_01.wav')
)

$sampleRate = 48000
$durationSeconds = 0.20
$sampleCount = [int]($sampleRate * $durationSeconds)
$raw = [double[]]::new($sampleCount)
$mixed = [double[]]::new($sampleCount)
$windows = [double[]]::new($sampleCount)
$random = [System.Random]::new(94117)
$twoPi = 2.0 * [Math]::PI
$lowNoise = 0.0

# A deterministic metal-on-metal hit-confirm tick. The inharmonic partials and
# very short high-passed noise transient keep it crisp without using recordings,
# downloaded samples, or third-party source audio.
for ($index = 0; $index -lt $sampleCount; ++$index) {
    $time = $index / [double]$sampleRate
    $noise = $random.NextDouble() * 2.0 - 1.0
    $lowNoise += 0.18 * ($noise - $lowNoise)
    $highNoise = $noise - $lowNoise

    $attack = [Math]::Min(1.0, $time / 0.00018)
    $clickEnvelope = [Math]::Exp(-$time / 0.0038)
    $ringEnvelope = [Math]::Exp(-$time / 0.027)
    $airEnvelope = [Math]::Exp(-$time / 0.011)

    $metal = (
        [Math]::Sin($twoPi * 1837.0 * $time + 0.35) * 0.42 +
        [Math]::Sin($twoPi * 2719.0 * $time + 1.10) * 0.30 +
        [Math]::Sin($twoPi * 3971.0 * $time + 2.25) * 0.20 +
        [Math]::Sin($twoPi * 5587.0 * $time + 0.80) * 0.12
    ) * $ringEnvelope

    $secondaryTime = $time - 0.013
    $secondary = 0.0
    if ($secondaryTime -ge 0.0) {
        $secondaryAttack = [Math]::Min(1.0, $secondaryTime / 0.00020)
        $secondaryEnvelope = [Math]::Exp(-$secondaryTime / 0.010)
        $secondary = (
            [Math]::Sin($twoPi * 3229.0 * $secondaryTime + 0.20) * 0.13 +
            $highNoise * 0.09
        ) * $secondaryEnvelope * $secondaryAttack
    }

    $raw[$index] = (
        $highNoise * $clickEnvelope * 0.40 +
        $highNoise * $airEnvelope * 0.08 +
        $metal +
        $secondary
    ) * $attack
}

# One-pole DC blocker, followed by short edge ramps. A weighted mean correction
# below removes the remaining numerical DC component without moving the first or
# final sample away from digital zero.
$previousInput = 0.0
$previousOutput = 0.0
$dcPole = 0.992
$startFadeSamples = [int](0.0007 * $sampleRate)
$endFadeSamples = [int](0.026 * $sampleRate)
for ($index = 0; $index -lt $sampleCount; ++$index) {
    $filtered = $raw[$index] - $previousInput + $dcPole * $previousOutput
    $previousInput = $raw[$index]
    $previousOutput = $filtered

    $window = 1.0
    if ($index -lt $startFadeSamples) {
        $window *= $index / [double]$startFadeSamples
    }
    if ($index -ge $sampleCount - $endFadeSamples) {
        $window *= ($sampleCount - 1 - $index) / [double]($endFadeSamples - 1)
    }

    $windows[$index] = $window
    $mixed[$index] = [Math]::Tanh($filtered * 1.18) * $window
}

$sum = 0.0
$windowSum = 0.0
for ($index = 0; $index -lt $sampleCount; ++$index) {
    $sum += $mixed[$index]
    $windowSum += $windows[$index]
}
$dcCorrection = if ($windowSum -gt 0.0) { $sum / $windowSum } else { 0.0 }

$peak = 0.0
for ($index = 0; $index -lt $sampleCount; ++$index) {
    $mixed[$index] -= $dcCorrection * $windows[$index]
    $peak = [Math]::Max($peak, [Math]::Abs($mixed[$index]))
}
$normalization = if ($peak -gt 0.0) { 0.84 / $peak } else { 1.0 }

$resolvedOutput = [System.IO.Path]::GetFullPath($OutputPath)
$outputDirectory = [System.IO.Path]::GetDirectoryName($resolvedOutput)
[System.IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

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

    foreach ($sample in $mixed) {
        $pcm = [int16][Math]::Round(
            [Math]::Clamp($sample * $normalization, -1.0, 1.0) * 32767.0)
        $writer.Write($pcm)
    }
}
finally {
    $writer.Dispose()
    $stream.Dispose()
}

Write-Output "Generated $resolvedOutput ($sampleRate Hz mono, 16-bit PCM, $durationSeconds s)"
