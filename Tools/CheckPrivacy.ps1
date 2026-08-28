[CmdletBinding()]
param(
    [string]$RepositoryRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Invoke-GitBytes
{
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments,

        [int[]]$AllowedExitCodes = @(0)
    )

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = 'git'
    $startInfo.WorkingDirectory = $script:ResolvedRepositoryRoot
    $startInfo.Arguments = [string]::Join(' ', $Arguments)
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.EnvironmentVariables['GIT_CONFIG_COUNT'] = '1'
    $startInfo.EnvironmentVariables['GIT_CONFIG_KEY_0'] = 'safe.directory'
    $startInfo.EnvironmentVariables['GIT_CONFIG_VALUE_0'] = $script:ResolvedRepositoryRoot

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    if (-not $process.Start())
    {
        throw 'Unable to start Git.'
    }

    $outputStream = [System.IO.MemoryStream]::new()
    try
    {
        $copyTask = $process.StandardOutput.BaseStream.CopyToAsync($outputStream)
        $errorTask = $process.StandardError.ReadToEndAsync()
        $process.WaitForExit()
        $null = $copyTask.GetAwaiter().GetResult()
        $null = $errorTask.GetAwaiter().GetResult()

        if ($AllowedExitCodes -notcontains $process.ExitCode)
        {
            throw ('Git command failed with exit code {0}.' -f $process.ExitCode)
        }

        return [pscustomobject]@{
            ExitCode = $process.ExitCode
            Bytes = $outputStream.ToArray()
        }
    }
    finally
    {
        $outputStream.Dispose()
        $process.Dispose()
    }
}

function ConvertFrom-StrictUtf8
{
    param([byte[]]$Bytes)

    $encoding = [System.Text.UTF8Encoding]::new($false, $true)
    return $encoding.GetString($Bytes)
}

function ConvertFrom-RepositoryText
{
    param([byte[]]$Bytes)

    if ($Bytes.Length -eq 0)
    {
        return ''
    }

    try
    {
        if ($Bytes.Length -ge 4 -and
            $Bytes[0] -eq 0xFF -and $Bytes[1] -eq 0xFE -and
            $Bytes[2] -eq 0x00 -and $Bytes[3] -eq 0x00)
        {
            return [System.Text.Encoding]::UTF32.GetString($Bytes, 4, $Bytes.Length - 4)
        }

        if ($Bytes.Length -ge 4 -and
            $Bytes[0] -eq 0x00 -and $Bytes[1] -eq 0x00 -and
            $Bytes[2] -eq 0xFE -and $Bytes[3] -eq 0xFF)
        {
            $encoding = [System.Text.UTF32Encoding]::new($true, $true, $true)
            return $encoding.GetString($Bytes, 4, $Bytes.Length - 4)
        }

        if ($Bytes.Length -ge 3 -and
            $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF)
        {
            $encoding = [System.Text.UTF8Encoding]::new($false, $true)
            return $encoding.GetString($Bytes, 3, $Bytes.Length - 3)
        }

        if ($Bytes.Length -ge 2 -and $Bytes[0] -eq 0xFF -and $Bytes[1] -eq 0xFE)
        {
            return [System.Text.Encoding]::Unicode.GetString($Bytes, 2, $Bytes.Length - 2)
        }

        if ($Bytes.Length -ge 2 -and $Bytes[0] -eq 0xFE -and $Bytes[1] -eq 0xFF)
        {
            return [System.Text.Encoding]::BigEndianUnicode.GetString(
                $Bytes, 2, $Bytes.Length - 2)
        }

        $text = ConvertFrom-StrictUtf8 -Bytes $Bytes
        if ($text.IndexOf([char]0) -ge 0)
        {
            return $null
        }
        return $text
    }
    catch [System.Text.DecoderFallbackException]
    {
        return $null
    }
}

function Test-RepositoryTextPath
{
    param([string]$Path)

    $fileName = [System.IO.Path]::GetFileName($Path).ToLowerInvariant()
    if ($script:TextFileNames -contains $fileName)
    {
        return $true
    }

    $extension = [System.IO.Path]::GetExtension($Path).ToLowerInvariant()
    return $script:TextExtensions -contains $extension
}

function Get-ObjectStringValues
{
    param([object]$Value)

    if ($null -eq $Value)
    {
        return
    }

    if ($Value -is [string])
    {
        Write-Output $Value
        return
    }

    if ($Value -is [System.Collections.IDictionary])
    {
        foreach ($dictionaryValue in $Value.Values)
        {
            Get-ObjectStringValues -Value $dictionaryValue
        }
        return
    }

    if ($Value -is [System.Collections.IEnumerable])
    {
        foreach ($item in $Value)
        {
            Get-ObjectStringValues -Value $item
        }
        return
    }

    foreach ($property in $Value.PSObject.Properties)
    {
        Get-ObjectStringValues -Value $property.Value
    }
}

function Get-LineNumber
{
    param(
        [string]$Text,
        [int]$CharacterIndex
    )

    if ($CharacterIndex -le 0)
    {
        return 1
    }

    $lineNumber = 1
    for ($index = 0; $index -lt $CharacterIndex; ++$index)
    {
        if ($Text[$index] -eq "`n")
        {
            ++$lineNumber
        }
    }
    return $lineNumber
}

function Add-PrivacyFinding
{
    param(
        [string]$Source,
        [string]$Category,
        [string]$Path,
        [int]$Line
    )

    $key = '{0}|{1}|{2}|{3}' -f $Source, $Category, $Path, $Line
    if ($script:FindingKeys.Add($key))
    {
        $script:Findings.Add([pscustomobject]@{
            Source = $Source
            Category = $Category
            Path = $Path
            Line = $Line
        })
    }
}

function Test-AllowedEmail
{
    param([string]$Email)

    if ($Email.Equals(
        'contributors@users.noreply.github.com',
        [System.StringComparison]::OrdinalIgnoreCase))
    {
        return $true
    }

    $atIndex = $Email.LastIndexOf('@')
    if ($atIndex -lt 0)
    {
        return $false
    }

    $domain = $Email.Substring($atIndex + 1).ToLowerInvariant()
    return @('example.com', 'example.org', 'example.net', 'invalid') -contains $domain
}

function Scan-TextSnapshot
{
    param(
        [string]$Source,
        [string]$Path,
        [string]$Text
    )

    for ($literalIndex = 0; $literalIndex -lt $script:BlockedLiterals.Count; ++$literalIndex)
    {
        $literal = $script:BlockedLiterals[$literalIndex]
        $matchIndex = $Text.IndexOf($literal, [System.StringComparison]::OrdinalIgnoreCase)
        if ($matchIndex -ge 0)
        {
            Add-PrivacyFinding -Source $Source `
                -Category ('configured local value #{0}' -f ($literalIndex + 1)) `
                -Path $Path -Line (Get-LineNumber -Text $Text -CharacterIndex $matchIndex)
        }
    }

    foreach ($rule in $script:BuiltInRules)
    {
        foreach ($match in $rule.Pattern.Matches($Text))
        {
            Add-PrivacyFinding -Source $Source -Category $rule.Category -Path $Path `
                -Line (Get-LineNumber -Text $Text -CharacterIndex $match.Index)
        }
    }

    foreach ($match in $script:EmailPattern.Matches($Text))
    {
        if (-not (Test-AllowedEmail -Email $match.Value))
        {
            Add-PrivacyFinding -Source $Source -Category 'email address' -Path $Path `
                -Line (Get-LineNumber -Text $Text -CharacterIndex $match.Index)
        }
    }
}

try
{
    if ([string]::IsNullOrWhiteSpace($RepositoryRoot))
    {
        if ([string]::IsNullOrWhiteSpace($PSScriptRoot))
        {
            throw 'Repository root was not supplied and the script directory is unavailable.'
        }

        $RepositoryRoot = Split-Path -Parent $PSScriptRoot
    }

    $script:ResolvedRepositoryRoot = [System.IO.Path]::GetFullPath($RepositoryRoot)
    if (-not (Test-Path -LiteralPath (Join-Path $script:ResolvedRepositoryRoot '.git')))
    {
        throw 'Repository root does not contain Git metadata.'
    }

    $script:TextFileNames = @(
        '.gitattributes', '.gitignore', '.vsconfig', 'license', 'license.txt',
        'notice', 'notice.txt'
    )
    $script:TextExtensions = @(
        '.c', '.cc', '.cpp', '.h', '.hpp', '.cs', '.ini', '.json', '.md',
        '.txt', '.csv', '.tsv', '.xml', '.yaml', '.yml', '.toml', '.ps1',
        '.psm1', '.py', '.uproject', '.uplugin', '.archive', '.po', '.manifest',
        '.props', '.targets', '.usf', '.ush', '.bat', '.cmd', '.sh'
    )

    $script:Findings = [System.Collections.Generic.List[object]]::new()
    $script:FindingKeys = [System.Collections.Generic.HashSet[string]]::new(
        [System.StringComparer]::Ordinal)

    $privateRelativePath = '.local/private.json'
    $privateFullPath = Join-Path $script:ResolvedRepositoryRoot '.local\private.json'

    $localOnlyIgnoreProbes = @(
        $privateRelativePath,
        '.local-licensed-assets/.privacy-check-probe',
        'Content/HumanVocalizations/.privacy-check-probe',
        'Content/InterfaceAndItemSounds/.privacy-check-probe',
        'Content/FPS_Weapon_Bundle/.privacy-check-probe'
    )
    foreach ($ignoreProbe in $localOnlyIgnoreProbes)
    {
        $ignoreResult = Invoke-GitBytes `
            -Arguments @('check-ignore', '-q', '--no-index', '--', $ignoreProbe) `
            -AllowedExitCodes @(0, 1)
        if ($ignoreResult.ExitCode -ne 0)
        {
            throw 'A local-only path is not protected by .gitignore.'
        }
    }

    $trackedPrivateResult = Invoke-GitBytes `
        -Arguments @('ls-files', '--error-unmatch', '--', $privateRelativePath) `
        -AllowedExitCodes @(0, 1)
    $privateFileIsTracked = $trackedPrivateResult.ExitCode -eq 0
    if ($privateFileIsTracked)
    {
        Add-PrivacyFinding -Source 'index' -Category 'local private file is tracked' `
            -Path $privateRelativePath -Line 0
    }

    $literalCandidates = @()
    if ((Test-Path -LiteralPath $privateFullPath -PathType Leaf) -and
        -not $privateFileIsTracked)
    {
        try
        {
            $privateConfigText = ConvertFrom-RepositoryText -Bytes (
                [System.IO.File]::ReadAllBytes($privateFullPath))
            if ($null -eq $privateConfigText)
            {
                throw 'Unsupported private configuration text encoding.'
            }
            $privateConfig = ConvertFrom-Json -InputObject $privateConfigText
        }
        catch
        {
            throw 'The local private file is not valid JSON.'
        }

        $localProfileProperty = $privateConfig.PSObject.Properties['localProfile']
        if ($null -ne $localProfileProperty)
        {
            $literalCandidates += @(Get-ObjectStringValues -Value $localProfileProperty.Value)
        }

        $privacyScanProperty = $privateConfig.PSObject.Properties['privacyScan']
        if ($null -ne $privacyScanProperty)
        {
            $blockedProperty = $privacyScanProperty.Value.PSObject.Properties['blockedLiterals']
            if ($null -ne $blockedProperty)
            {
                $literalCandidates += @(Get-ObjectStringValues -Value $blockedProperty.Value)
            }
        }
    }

    $literalSet = [System.Collections.Generic.HashSet[string]]::new(
        [System.StringComparer]::OrdinalIgnoreCase)
    $script:BlockedLiterals = [System.Collections.Generic.List[string]]::new()
    $candidateNumber = 0
    foreach ($candidate in $literalCandidates)
    {
        ++$candidateNumber
        $literal = ([string]$candidate).Trim()
        if ([string]::IsNullOrWhiteSpace($literal))
        {
            continue
        }
        if ($literal.IndexOfAny(@([char]0, [char]10, [char]13)) -ge 0)
        {
            throw ('Local privacy entry #{0} contains a control character.' -f $candidateNumber)
        }

        $minimumLength = if ($literal -match '[^\x00-\x7F]') { 2 } else { 4 }
        if ($literal.Length -lt $minimumLength)
        {
            throw ('Local privacy entry #{0} is too short for safe literal matching.' -f $candidateNumber)
        }

        if ($literalSet.Add($literal))
        {
            $script:BlockedLiterals.Add($literal)
        }
    }

    $regexOptions = [System.Text.RegularExpressions.RegexOptions]::Compiled -bor
        [System.Text.RegularExpressions.RegexOptions]::CultureInvariant
    $script:BuiltInRules = @(
        [pscustomobject]@{
            Category = 'Windows user profile path'
            Pattern = [regex]::new(
                '(?i)\b[A-Z]:[\\/]+(?:Users|Documents and Settings)[\\/]+(?![%<{])[^\\/\r\n\s"<>]+',
                $regexOptions)
        },
        [pscustomobject]@{
            Category = 'Unix user profile path'
            Pattern = [regex]::new(
                '(?i)(?<![A-Z0-9])/(?:Users|home)/(?![%<{])[^/\r\n\s"<>]+',
                $regexOptions)
        },
        [pscustomobject]@{
            Category = 'credential-bearing URL'
            Pattern = [regex]::new(
                '(?i)\bhttps?://[^\s/@:]+:[^\s/@]+@', $regexOptions)
        },
        [pscustomobject]@{
            Category = 'credential in URL query'
            Pattern = [regex]::new(
                '(?i)[?&](?:access[_-]?token|api[_-]?key|password|passwd|secret)=(?!<|%7B|\$\{)[^&\s"''<>]+',
                $regexOptions)
        },
        [pscustomobject]@{
            Category = 'private key material'
            Pattern = [regex]::new(
                '-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----', $regexOptions)
        },
        [pscustomobject]@{
            Category = 'GitHub access token'
            Pattern = [regex]::new(
                '(?i)(?:github_pat_[A-Z0-9_]{20,}|gh[pousr]_[A-Z0-9]{20,})',
                $regexOptions)
        },
        [pscustomobject]@{
            Category = 'cloud access token'
            Pattern = [regex]::new(
                '(?:AKIA[0-9A-Z]{16}|AIza[0-9A-Za-z_-]{30,}|glpat-[0-9A-Za-z_-]{20,})',
                $regexOptions)
        },
        [pscustomobject]@{
            Category = 'machine-specific Unreal Engine association'
            Pattern = [regex]::new(
                '(?im)"EngineAssociation"\s*:\s*"(?!\s*")[^"]+"',
                $regexOptions)
        },
        [pscustomobject]@{
            Category = 'non-empty Android file-server token'
            Pattern = [regex]::new(
                '(?im)^\s*SecurityToken\s*=\s*\S+', $regexOptions)
        }
    )
    $script:EmailPattern = [regex]::new(
        '(?i)(?<![A-Z0-9._%+-])[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}(?![A-Z0-9.-])',
        $regexOptions)

    $allowedGitName = 'FPSPrototype Contributors'
    $allowedGitEmail = 'contributors@users.noreply.github.com'
    $gitNameResult = Invoke-GitBytes `
        -Arguments @('config', '--local', '--get', 'user.name') `
        -AllowedExitCodes @(0, 1)
    $gitEmailResult = Invoke-GitBytes `
        -Arguments @('config', '--local', '--get', 'user.email') `
        -AllowedExitCodes @(0, 1)
    $gitName = (ConvertFrom-StrictUtf8 -Bytes $gitNameResult.Bytes).Trim()
    $gitEmail = (ConvertFrom-StrictUtf8 -Bytes $gitEmailResult.Bytes).Trim()
    if (-not $gitName.Equals($allowedGitName, [System.StringComparison]::Ordinal) -or
        -not $gitEmail.Equals($allowedGitEmail, [System.StringComparison]::OrdinalIgnoreCase))
    {
        Add-PrivacyFinding -Source 'git-config' -Category 'non-generic commit identity' `
            -Path '.git/config' -Line 0
    }

    $indexResult = Invoke-GitBytes -Arguments @('ls-files', '--stage', '-z')
    $indexListing = ConvertFrom-StrictUtf8 -Bytes $indexResult.Bytes
    $indexEntries = [System.Collections.Generic.List[object]]::new()
    $worktreePaths = [System.Collections.Generic.HashSet[string]]::new(
        [System.StringComparer]::Ordinal)
    foreach ($record in $indexListing.Split([char]0))
    {
        if ([string]::IsNullOrEmpty($record))
        {
            continue
        }

        $tabIndex = $record.IndexOf("`t")
        if ($tabIndex -lt 0)
        {
            throw 'Unable to parse the Git index listing.'
        }
        $metadata = $record.Substring(0, $tabIndex).Split(' ')
        if ($metadata.Count -ne 3)
        {
            throw 'Unable to parse Git index metadata.'
        }

        $path = $record.Substring($tabIndex + 1)
        $entry = [pscustomobject]@{
            Mode = $metadata[0]
            ObjectId = $metadata[1]
            Stage = [int]$metadata[2]
            Path = $path
        }
        $indexEntries.Add($entry)
        $null = $worktreePaths.Add($path)
    }

    $localOnlyAssetPrefixes = @(
        '.local-licensed-assets/',
        'Content/HumanVocalizations/',
        'Content/InterfaceAndItemSounds/',
        'Content/FPS_Weapon_Bundle/'
    )
    foreach ($entry in $indexEntries)
    {
        foreach ($localOnlyPrefix in $localOnlyAssetPrefixes)
        {
            if ($entry.Path.StartsWith(
                $localOnlyPrefix, [System.StringComparison]::OrdinalIgnoreCase))
            {
                Add-PrivacyFinding -Source 'index' `
                    -Category 'local-only licensed asset is tracked' `
                    -Path $entry.Path -Line 0
            }
        }
    }

    $skippedTextFiles = 0
    foreach ($entry in $indexEntries)
    {
        if ($entry.Mode -eq '160000' -or -not (Test-RepositoryTextPath -Path $entry.Path))
        {
            continue
        }

        $blobResult = Invoke-GitBytes -Arguments @('cat-file', 'blob', $entry.ObjectId)
        $text = ConvertFrom-RepositoryText -Bytes $blobResult.Bytes
        if ($null -eq $text)
        {
            ++$skippedTextFiles
            continue
        }

        $source = if ($entry.Stage -eq 0) { 'index' } else { 'index-stage-{0}' -f $entry.Stage }
        Scan-TextSnapshot -Source $source -Path $entry.Path -Text $text
    }

    $rootWithSeparator = $script:ResolvedRepositoryRoot.TrimEnd('\', '/') +
        [System.IO.Path]::DirectorySeparatorChar
    foreach ($path in $worktreePaths)
    {
        if (-not (Test-RepositoryTextPath -Path $path))
        {
            continue
        }

        $fullPath = [System.IO.Path]::GetFullPath((Join-Path $script:ResolvedRepositoryRoot $path))
        if (-not $fullPath.StartsWith(
            $rootWithSeparator, [System.StringComparison]::OrdinalIgnoreCase))
        {
            throw 'A tracked path resolves outside the repository root.'
        }
        if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf))
        {
            continue
        }
        if ((Get-Item -LiteralPath $fullPath).Attributes -band
            [System.IO.FileAttributes]::ReparsePoint)
        {
            throw 'A tracked text path is a reparse point and cannot be scanned safely.'
        }

        $text = ConvertFrom-RepositoryText -Bytes ([System.IO.File]::ReadAllBytes($fullPath))
        if ($null -eq $text)
        {
            ++$skippedTextFiles
            continue
        }
        Scan-TextSnapshot -Source 'worktree' -Path $path -Text $text
    }

    if ($script:Findings.Count -gt 0)
    {
        Write-Output ('PRIVACY_CHECK_FAILED Findings={0}' -f $script:Findings.Count)
        foreach ($finding in ($script:Findings | Sort-Object Source, Path, Line, Category))
        {
            $location = if ($finding.Line -gt 0)
            {
                '{0}:{1}' -f $finding.Path, $finding.Line
            }
            else
            {
                $finding.Path
            }
            Write-Output ('[{0}] {1} - {2}' -f
                $finding.Source, $location, $finding.Category)
        }
        exit 1
    }

    Write-Output ('PRIVACY_CHECK_OK TrackedEntries={0} LocalLiterals={1} SkippedText={2}' -f
        $indexEntries.Count, $script:BlockedLiterals.Count, $skippedTextFiles)
    exit 0
}
catch
{
    Write-Verbose ('Privacy checker internal detail: {0}: {1}' -f
        $_.Exception.GetType().Name, $_.Exception.Message)
    Write-Error `
        'Privacy check could not complete safely. Review the local configuration and Git state.' `
        -ErrorAction Continue
    exit 2
}
