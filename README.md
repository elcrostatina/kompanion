# Kompanion

An AI chat sidebar for KDE Plasma 6, docked to the **left** (or, if you prefer, **right**) edge of the screen, in the spirit of the Windows Copilot panel.
It puts two kinds of providers in one place:

* **Web providers** – the official web apps of Claude, ChatGPT, Microsoft Copilot, Gemini, Proton Lumo and any custom URL, shown in an
  embedded Chromium view, with an optional local CSS/JS override per provider.
* **Local providers** – models on this machine or your network behind an OpenAI-compatible API (LM Studio, Ollama, …) with a
  native chat UI and chats stored locally.

No telemetry, no update checks. Nothing is sent anywhere except to the providers you configure.

App ID: `io.github.elcrostatina.Kompanion` · Config file: `kompanionrc` · License: GPL-3.0-or-later

## Features

Panel
* Full-height overlay anchored to the left edge (LayerShellQt, overlay layer, exclusive zone 0, on-demand keyboard focus);
  frameless always-on-top window at the screen edge on X11. Settings › “Open on” switches between the left and the right edge, and “Style” can detach the panel from the screen edges (a floating panel with rounded corners, like a floating Plasma panel; the sidebar and the content become two separate rounded cards with a small gap between them, like two tiled windows).
* Default width 440 px, resizable by dragging the inner edge, width and side remembered.
* Show/hide with a global shortcut (default **Meta+C**, configurable in System Settings › Shortcuts). On first run you are
  warned if the shortcut is already taken. Slides in/out in 150 ms, **Esc** hides it.
* Hiding never unloads pages, so conversations keep running.
* Tray icon (show/hide, switch provider, quit), optional autostart, `kompanion --settings` opens the settings page.
* Vertical sidebar rail with tinted rounded-tile provider buttons, grouped into Web and Local, a sliding highlight for the active provider and a button to expand it to show names (state remembered); switching providers and opening Settings use short slide-and-fade transitions; **Ctrl+1 … Ctrl+9** switch providers and the last used provider is remembered.

Web providers
* One persistent WebEngine profile per provider under `$XDG_DATA_HOME/kompanion/profiles/<id>/`: logins survive restarts
  and providers stay isolated.
* Current Chrome user agent derived from the bundled engine (editable per provider).
* Per-provider zoom (remembered), back, reload, open in browser, downloads to `~/Downloads`, file upload,
  clipboard, microphone/camera prompts (decision remembered per provider), context menu.
* Links that leave the provider's site open in the system browser (sign-in hosts such as Google, Microsoft, Apple and
  OpenAI stay inside the panel).
* “Clear cookies, cache and site data” per provider.
* The panel chrome adapts to the page: the toolbar under a web view takes the background colour of the site (sampled from the page, so it works with every site and with light or dark themes), and scrollbars are thinned by a small built-in base style that your own CSS can override.
* Local style overrides with live reload (see below).

Local providers
* `/v1/models` model picker, streamed answers (SSE) with a Stop button, Markdown rendering, syntax-highlighted code blocks
  with a Copy button.
* Chat list with new, rename, delete and search over titles **and** message text; titles are generated from the first message.
* System prompt per chat plus a global default; Enter sends, Shift+Enter adds a new line.
* Clear errors when the server is unreachable, the API key is rejected or no model is loaded.
* Presets for LM Studio (`http://localhost:1234/v1`) and Ollama (`http://localhost:11434/v1`) plus custom URLs (LAN servers,
  optional API key stored in the wallet).
* Export a chat to Markdown or JSON, import from JSON. Storage: SQLite at `$XDG_DATA_HOME/kompanion/chats.db`.

## Dependencies

Qt ≥ 6.6, KDE Frameworks ≥ 6.8, CMake ≥ 3.20, a C++20 compiler. Runtime: Kirigami and the Plasma Qt Quick style
(`qqc2-desktop-style`) and the Qt Quick Effects module (part of qtdeclarative; used for the rounded corners of the floating style). Package names differ a little between releases; look up the right one if a package is missing.

**KDE neon / Ubuntu** (the command below was verified on KDE neon 24.04; stock Ubuntu needs the KDE neon or a newer Plasma archive for KF 6.8+)

```bash
sudo apt install build-essential cmake ninja-build git extra-cmake-modules \
  qt6-base-dev qt6-base-private-dev qt6-declarative-dev qt6-webengine-dev qt6-tools-dev qt6-wayland-dev \
  kf6-kirigami-dev qml6-module-org-kde-kirigami libkf6config-dev libkf6i18n-dev libkf6globalaccel-dev \
  kf6-kstatusnotifieritem-dev libkf6kio-dev libkf6coreaddons-dev libkf6windowsystem-dev \
  libkf6syntaxhighlighting-dev liblayershellqtinterface-dev qtkeychain-qt6-dev
```

**Fedora**

```bash
sudo dnf install gcc-c++ cmake ninja-build extra-cmake-modules qt6-qtbase-devel qt6-qtdeclarative-devel \
  qt6-qtwebengine-devel qt6-qttools-devel kf6-kirigami-devel kf6-kconfig-devel kf6-ki18n-devel \
  kf6-kglobalaccel-devel kf6-kstatusnotifieritem-devel kf6-kio-devel kf6-kcoreaddons-devel \
  kf6-kwindowsystem-devel kf6-kdbusaddons-devel kf6-syntax-highlighting-devel layer-shell-qt-devel \
  qtkeychain-qt6-devel kf6-qqc2-desktop-style
```

**Arch Linux**

```bash
sudo pacman -S base-devel cmake ninja extra-cmake-modules qt6-base qt6-declarative qt6-webengine qt6-tools \
  kirigami kconfig ki18n kglobalaccel kstatusnotifieritem kio kcoreaddons kwindowsystem kdbusaddons \
  syntax-highlighting layer-shell-qt qtkeychain-qt6 qqc2-desktop-style
```

**openSUSE Tumbleweed**

```bash
sudo zypper install gcc-c++ cmake ninja extra-cmake-modules qt6-base-devel qt6-declarative-devel \
  qt6-webengine-devel qt6-tools-devel kf6-kirigami-devel kf6-kconfig-devel kf6-ki18n-devel \
  kf6-kglobalaccel-devel kf6-kstatusnotifieritem-devel kf6-kio-devel kf6-kcoreaddons-devel \
  kf6-kwindowsystem-devel kf6-kdbusaddons-devel kf6-syntax-highlighting-devel layer-shell-qt6-devel \
  qtkeychain-qt6-devel kf6-qqc2-desktop-style
```

## Build

```bash
cmake -B build && cmake --build build
ctest --test-dir build --output-on-failure
./build/bin/kompanion
```

Install with `sudo cmake --install build` to get the desktop entry, icon and AppStream metadata. Running the binary a second
time toggles the panel (it is a single-instance application), so you can also bind `kompanion` to any key.

## Flatpak and downloadable builds

`io.github.elcrostatina.Kompanion.yml` is the Flatpak manifest (KDE runtime 6.11 with the Qt WebEngine base app; QtKeychain and
LayerShellQt are built as modules). The GitHub Actions workflow in `.github/workflows/flatpak.yml` builds it for x86_64 and
aarch64 on every push to `main`, on pull requests, on tags and on demand:

* **Every run:** open the run in the *Actions* tab and download `kompanion-x86_64.flatpak` or `kompanion-aarch64.flatpak` from
  the *Artifacts* section.
* **Tags:** pushing a tag such as `v0.1.0` also attaches both bundles to a GitHub release.

Install a downloaded bundle with:

```bash
flatpak install --user ./kompanion-x86_64.flatpak
flatpak run io.github.elcrostatina.Kompanion
```

To build the same bundle locally:

```bash
flatpak install flathub org.kde.Platform//6.11 org.kde.Sdk//6.11 io.qt.qtwebengine.BaseApp//6.11
flatpak-builder --user --install --force-clean build-flatpak io.github.elcrostatina.Kompanion.yml
```

Inside the sandbox the settings and override files live under `~/.var/app/io.github.elcrostatina.Kompanion/config/kompanion/`,
and the “Autostart” switch cannot write to the host's autostart folder (use the desktop's own autostart settings instead).
Use `-DKOMPANION_BUILD_TESTS=OFF` to skip the unit tests in a regular CMake build.

See [RELEASING.md](RELEASING.md) for how to commit, trigger the pipeline by hand and publish a release.

## Style overrides

Every web provider reads two optional files from `$XDG_CONFIG_HOME/kompanion/overrides/` (usually `~/.config/kompanion/overrides/`):

| File | Purpose |
|------|---------|
| `<provider-id>.css` | Injected as a `<style>` element (id `kompanion-override-style`). Created from the shipped default the first time the provider is used. |
| `<provider-id>.js`  | Optional JavaScript, run once per page load in the page's main world, wrapped in `try/catch`. |

The provider id is the lower-case name used in `kompanionrc` (`claude`, `chatgpt`, `copilot`, `gemini`, `lumo`, or a slug such as
`my-site` for custom providers). Both files are injected at `DocumentReady` and again on every navigation. The style element
is re-added if a page removes it, so it survives single-page apps that rebuild the DOM. The files are watched: **saving the CSS
file re-styles the open page immediately**; JavaScript changes apply on the next load.

In Settings, each web provider has a switch to turn overrides on or off, **Edit CSS** / **Edit JS** (opens your text editor)
and **Reset** (restores the shipped default CSS). The defaults live in `data/overrides/`; they are deliberately small because
the sites change their markup often. A narrow panel already makes most sites switch to their mobile layout, so the defaults
mostly trim padding and prevent horizontal scrolling.

Finding selectors: start Kompanion with `QTWEBENGINE_REMOTE_DEBUGGING=9222 kompanion` and open `http://localhost:9222`
in a Chromium-based browser to inspect the panel's pages.

```css
nav[aria-label="Sidebar"] { display: none !important; }
main { padding-inline: 4px !important; }
```

## Choices made along the way

* **Full height** means the full height of the area not claimed by other panels: the layer-shell exclusive zone is 0, so
  Plasma panels keep their space and the sidebar stops at them.
* **Dark/light for web pages**: Qt WebEngine already reports Plasma's color scheme through `prefers-color-scheme`. The setting
  “Match the Plasma dark/light color scheme” (on by default) only matters when turned off: pages are then told the scheme is light
  (applied at the next start).
* **Sign-in hosts** (Google, Microsoft, Apple, OpenAI, GitHub) are allowed to navigate inside the panel, because blocking them
  would make signing in impossible. Everything else that a user clicks and that leaves the provider's site opens in the browser.
* **Chats**: the most recent chat of a provider is reopened on start; “New chat” starts a draft that is only stored once you send.
  Titles come from the first message (no extra model call).
* **API keys** go through QtKeychain (KWallet on Plasma, Secret Service elsewhere) behind the small `SecretStore` interface.
* **Presets** are added once: when a new release brings a new preset (such as Lumo), it is appended to existing installs; a preset you removed is never brought back. New providers are inserted after the last provider of the same kind, so the rail groups stay together.
* The floating style has no drop shadow, because layer-shell surfaces cannot draw outside their own area; it uses a thin border instead.
* The code highlighter is called `CodeBlockHighlighter` rather than `MarkdownHighlighter`, because it highlights fenced code.
* Messages are modelled as blocks (text or code), so streaming only touches the last block instead of rebuilding the list.

## Known limitations

* **Sign-in can still fail inside the panel.** Some identity providers and bot checks (Cloudflare, Google) refuse embedded
  browsers, even with a Chrome user agent. Use the “Open sign-in page in your browser” button, but note that the system browser
  does not share cookies with the panel, so this only helps for flows that complete with an emailed code or link. Pop-up based
  OAuth windows are loaded in the same view, so `window.opener` flows do not work.
* Wayland only gets the real overlay behaviour where the compositor supports `wlr-layer-shell` (KWin does; GNOME/Mutter does
  not). Elsewhere you get an ordinary frameless window. The X11 fallback has not been exercised yet.
* On-demand keyboard focus: if the panel does not take focus when shown, click into it once.
* While an answer is streaming you cannot switch chats or start a new one; press Stop first.
* No token counting or history trimming for local chats: the whole chat is sent with every request.
* “Clear site data” removes cookies and the HTTP cache at once and wipes the rest of the profile directory the next time Kompanion
  starts.
* Reordering or disabling providers rebuilds the provider view, so web pages that were already open reload.
* Default CSS overrides are best-effort and may need tweaking after a site redesign.
* No translations are shipped yet (strings are wrapped for KI18n).
* The bundled Qt WebEngine has no proprietary codecs; voice features of some sites may be limited.
