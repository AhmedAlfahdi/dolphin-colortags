# Dolphin Color Tags — KDE Store listing

Use these values in the Add Product form at <https://store.kde.org>.

## Fields

| Field | Value |
|-------|-------|
| Name | `Dolphin Color Tags` |
| Summary / tagline | `Color tags for files and folders in Dolphin, with colored icon badges and a popup palette.` |
| Category | `Dolphin Service Menus` (the Store's Dolphin section, cat 102) |
| Version | `1.0.0` |
| License | `GPL-2.0-or-later` |
| Price | Free |
| Homepage | `https://github.com/AhmedAlfahdi/dolphin-colortags` |
| Source code | `https://github.com/AhmedAlfahdi/dolphin-colortags` (also bundled in the download) |
| Download file | `dist/dolphin-colortags-1.0.0.tar.gz` |
| Release page | `https://github.com/AhmedAlfahdi/dolphin-colortags/releases/tag/v1.0.0` |
| Direct download (mirror) | `https://github.com/AhmedAlfahdi/dolphin-colortags/releases/download/v1.0.0/dolphin-colortags-1.0.0.tar.gz` |
| Screenshot 1 | `preview-1.png` (first: becomes the thumbnail) |
| Screenshot 2 | `preview-2.png` |
| Tags | `dolphin`, `color tags`, `folder color`, `file manager`, `kde`, `plasma6`, `plugin`, `overlay icons` |

## Changelog

See `changelog.txt`.

## Short description (if a separate short-description field is requested)

> Seven-color tags for files and folders in Dolphin. Right-click → Color Tag;
> the color appears as a dot on the icon and refreshes instantly.

## Notes for the moderator

- This is a native Dolphin plugin (KOverlayIconPlugin +
  KAbstractFileItemActionPlugin), not a `.desktop` service menu. The Store has
  no separate "Dolphin plugins" category, so it is filed under Dolphin Service
  Menus, where the rest of the Dolphin add-ons live.
- It targets Dolphin / Plasma 6 with KDE Frameworks 6. It does not work on
  Plasma 5.
- Tags are stored in the `user.colortag` extended attribute; no service or
  daemon is started.
- The download is a source archive with an interactive `install.sh`; build
  requirements are documented in the description and README.
- License is GPL-2.0-or-later; full source is in the archive.
