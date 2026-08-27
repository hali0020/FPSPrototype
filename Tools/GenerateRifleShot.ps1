param(
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\SourceAssets\Audio\SFX_Rifle_Shot_01.wav')
)

$sampleRate = 48000
$durationSeconds = 0.82
$sampleCount = [int]($sampleRate * $durationSeconds)
$direct = [double[]]::new($sampleCount)
$mixed = [double[]]::new($sampleCount)
$random = [System.Random]::new(76241)
$twoPi = 2.0 * [Math]::PI
$lowNoise = 0.0
$previousNoise = 0.0

# A deterministic, license-free rifle report made from a short supersonic crack,
# broadband muzzle blast, low-frequency pressure wave, action click and early
# reflections. It intentionally contains no downloaded or recorded source audio.
for ($index = 0; $index -lt $sampleCount; ++$index) {
    $time = $index / [double]$sampleRate
    $noise = $random.NextDouble() * 2.0 - 1.0
    $lowNoise += 0.065 * ($noise - $lowNoise)
    $highNoise = $noise - 0.72 * $previousNoise
    $previousNoise = $noise

    $crackEnvelope = [Math]::Exp(-$time / 0.010)
    $blastEnvelope = [Math]::Exp(-$time / 0.052)
    $bodyEnvelope = [Math]::Exp(-$time / 0.145)
    $tailEnvelope = if ($time -gt 0.035) {
        [Math]::Exp(-($time - 0.035) / 0.23)
    } else {
        0.0
    }

    $sweepPhase = $twoPi * (118.0 * $time - 42.0 * $time * $time)
    $boom = [Math]::Sin($sweepPhase) * $bodyEnvelope * 0.58
    $pressure = [Math]::Sin($twoPi * 72.0 * $time) * [Math]::Exp(-$time / 0.19) * 0.20
    $crack = $highNoise * $crackEnvelope * 1.15
    $blast = ($noise * 0.42 + $lowNoise * 0.82) * $blastEnvelope
    $tail = ($noise * 0.08 + $lowNoise * 0.16) * $tailEnvelope

    $boltTime = $time - 0.067
    $boltEnvelope = [Math]::Exp(-[Math]::Pow($boltTime / 0.0045, 2.0))
    $bolt = ($highNoise * 0.42 + [Math]::Sin($twoPi * 1650.0 * $time) * 0.10) * $boltEnvelope

    # Start at digital zero and reach the full transient in 0.25 ms. This keeps
    # the muzzle crack sharp without introducing a one-sample import pop.
    $attack = [Math]::Min(1.0, $time / 0.00025)
    $direct[$index] = ($crack + $blast + $boom + $pressure + $tail + $bolt) * $attack
}

$reflectionTaps = @(
    @{ Delay = 0.000; Gain = 1.00 },
    @{ Delay = 0.041; Gain = 0.25 },
    @{ Delay = 0.079; Gain = -0.17 },
    @{ Delay = 0.126; Gain = 0.12 },
    @{ Delay = 0.193; Gain = -0.075 },
    @{ Delay = 0.287; Gain = 0.045 }
)

for ($index = 0; $index -lt $sampleCount; ++$index) {
    $value = 0.0
    foreach ($tap in $reflectionTaps) {
        $delaySamples = [int]($tap.Delay * $sampleRate)
        $sourceIndex = $index - $delaySamples
        if ($sourceIndex -ge 0) {
            $value += $direct[$sourceIndex] * $tap.Gain
        }
    }

    $fadeStart = $sampleCount - [int](0.045 * $sampleRate)
    if ($index -gt $fadeStart) {
        $value *= ($sampleCount - $index) / [double]($sampleCount - $fadeStart)
    }
    $mixed[$index] = [Math]::Tanh($value * 1.32)
}

$peak = 0.0
foreach ($sample in $mixed) {
    $peak = [Math]::Max($peak, [Math]::Abs($sample))
}
$normalization = if ($peak -gt 0.0) { 0.94 / $peak } else { 1.0 }

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
        $pcm = [int16][Math]::Round([Math]::Clamp($sample * $normalization, -1.0, 1.0) * 32767.0)
        $writer.Write($pcm)
    }
}
finally {
    $writer.Dispose()
    $stream.Dispose()
}

Write-Output "Generated $resolvedOutput ($sampleRate Hz mono, $durationSeconds s)"
