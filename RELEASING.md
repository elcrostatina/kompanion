# Committing, running the pipeline and releasing

The repository is `https://github.com/elcrostatina/kompanion` and the default branch is `main`.
The pipeline is `.github/workflows/flatpak.yml`; it builds the Flatpak bundles described by
`io.github.elcrostatina.Kompanion.yml`.

## 1. Check what will be committed

```bash
git status
```

Hidden files are easy to miss. `.gitignore` (keeps `build/` out of the repository), `.github/` (the pipeline) and `.reuse/`
(license metadata) must be committed, otherwise GitHub never sees the workflow. Add `.gitignore` before anything else, so a
broad `git add` cannot pick up `build/`.

```bash
git add .gitignore
git add .github .reuse io.github.elcrostatina.Kompanion.yml RELEASING.md README.md CMakeLists.txt
git status
```

Everything else in `git status` that is not meant to be committed should stay unstaged. Check that no path starts with
`build/` before continuing.

## 2. Check locally

```bash
cmake -B build && cmake --build build
ctest --test-dir build --output-on-failure
```

## 3. Commit and push (this triggers the pipeline)

```bash
git commit -m "ci: add the Flatpak workflow"
git push origin main
```

A push to `main` starts the workflow. The same happens for pull requests and for tags that start with `v`.

To work on a branch and let the pipeline check it first:

```bash
git switch -c my-change
git commit -am "Describe the change"
git push -u origin my-change
gh pr create --fill
```

## 4. Start the pipeline by hand

Either open the repository on GitHub, go to **Actions › Flatpak › Run workflow**, or use the GitHub CLI:

```bash
gh workflow run flatpak.yml --ref main
```

The workflow file must already exist on the chosen branch, which is why the first push in step 3 comes first.

## 5. Follow the run and download the bundles

```bash
gh run list --workflow=flatpak.yml
gh run watch
gh run view --log-failed
gh run download --name kompanion-x86_64.flatpak
```

Without the CLI: **Actions › the run › Artifacts** at the bottom of the page lists `kompanion-x86_64.flatpak` and
`kompanion-aarch64.flatpak`. Install one with:

```bash
flatpak install --user ./kompanion-x86_64.flatpak
flatpak run io.github.elcrostatina.Kompanion
```

The first build takes a while because the KDE runtime and the Qt WebEngine base app must be downloaded; later runs reuse the
cache. To retry a failed run: `gh run rerun --failed`.

## 6. Publish a release

1. Raise the version in `CMakeLists.txt` (`project(Kompanion VERSION x.y.z ...)`) and add a `<release version="x.y.z" date="YYYY-MM-DD"/>`
   line at the top of `<releases>` in `io.github.elcrostatina.Kompanion.metainfo.xml`.
2. Commit, then tag and push the tag:

```bash
git commit -am "Release x.y.z"
git push origin main
git tag -a vx.y.z -m "Kompanion x.y.z"
git push origin vx.y.z
```

The tag starts the workflow; when both bundles are built, a second job attaches them to a GitHub release named after the tag.
Look at it with `gh release view vx.y.z`. To redo a release, delete the release and the tag
(`gh release delete vx.y.z --cleanup-tag`) and tag again.

## 7. If something does not work

* **No “Actions” tab or no runs:** Actions are disabled for the repository. Enable them in *Settings › Actions › General*.
* **The aarch64 job never starts or is refused:** GitHub's hosted ARM runners (`ubuntu-24.04-arm`) are free for public repositories
  only. Remove the `aarch64` entry from `strategy.matrix.include` in the workflow for a private repository.
* **The release job fails with a permission error:** allow workflows to write in *Settings › Actions › General › Workflow
  permissions*, or keep the default and rely on the `contents: write` permission already set on that job.
* **The Flatpak build fails on a module:** the log (`gh run view --log-failed`) names it. LayerShellQt and QtKeychain are pinned to
  a tag and a commit in the manifest; change both together when updating them.
