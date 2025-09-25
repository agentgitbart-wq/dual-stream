# Script to rename all CLAUDE.md files to ARCHITECTURE.md and update references
# It commits and pushes changes on the current branch.

$ErrorActionPreference = 'Stop'

Write-Host "Searching for CLAUDE.md files..."
$files = Get-ChildItem -Path . -Recurse -Filter 'CLAUDE.md' -File -Force -ErrorAction SilentlyContinue
if (-not $files) {
    Write-Host "No CLAUDE.md files found. Exiting."
    exit 0
}

foreach ($f in $files) {
    $old = $f.FullName
    $new = Join-Path $f.DirectoryName 'ARCHITECTURE.md'
    Write-Host "Renaming: $old -> $new"
    git mv -f -- "$old" "$new"
}

# Replace textual references to CLAUDE.md in common text/source files
$exts = @('.md','.txt','.mdown','.markdown','.json','.cpp','.h','.hpp','.c','.cc','.cs','.py','.cmake','.yml','.yaml','.ini')
$filesToEdit = Get-ChildItem -Path . -Recurse -File -Force | Where-Object { ($exts -contains $_.Extension.ToLower()) -and ($_.FullName -notmatch '\\.git\\') }

foreach ($file in $filesToEdit) {
    $path = $file.FullName
    try {
        $content = Get-Content -Raw -Encoding UTF8 -ErrorAction Stop $path
    } catch {
        # skip files we can't read as text
        continue
    }
    $newContent = $content -replace '(?i)\bCLAUDE\.md\b','ARCHITECTURE.md'
    if ($newContent -ne $content) {
        Write-Host "Updating references in: $path"
        Set-Content -Encoding UTF8 -Value $newContent -Path $path
        git add -- "$path"
    }
}

# Finalize commit
Write-Host "Staging remaining changes and committing..."

git add -A
$branch = (git rev-parse --abbrev-ref HEAD).Trim()
Write-Host "Current branch: $branch"
try {
    git commit -m "chore: replace CLAUDE.md with ARCHITECTURE.md and update references"
} catch {
    Write-Host "Nothing to commit or commit failed: $_"
}

Write-Host "Pushing branch..."
git push -u origin $branch

Write-Host "Done."
