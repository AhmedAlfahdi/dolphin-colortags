# KDE Store submission kit — Dolphin Color Tags 1.0.0

Everything needed to publish the plugin on <https://store.kde.org> is in this
directory. Uploading itself needs a logged-in Store account; there is no usable
API any more (the OCS endpoint answers `410 This API endpoint is no longer
supported`).

## Files

| File | Use |
|------|-----|
| `listing.md` | Values to copy into the Add Product form |
| `description.html` | Long description, HTML version |
| `description.txt` | Long description, plain-text fallback |
| `changelog.txt` | Changelog field |
| `preview-1.png` | Screenshot 1: tags in Dolphin |
| `preview-2.png` | Screenshot 2: the palette popup |
| `dist/dolphin-colortags-1.0.0.tar.gz` | Download archive to upload |
| `dist/SHA256SUMS` | Checksum for the archive |

`dist/` is generated and not committed. Run `./make.sh` here to recreate it.

## Upload steps

1. Log in at <https://store.kde.org> (top right). A KDE Identity or
   OpenDesktop account works.
2. Open your account menu and choose **Add Product** (some accounts see
   **Submit Content**). The direct `/content/add` URL returns 404 while logged
   out, so start from the menu.
3. Category: **Dolphin Service Menus** (`cat=102`). This is the Store's only
   Dolphin section. The product is a native Dolphin plugin, not a `.desktop`
   service menu; say so in the first line of the description.
4. Fill the fields from `listing.md`.
5. Description: switch the editor to HTML/source mode and paste
   `description.html`. If the editor only accepts plain text, paste
   `description.txt`.
6. Upload `dist/dolphin-colortags-1.0.0.tar.gz` as the download file.
7. Upload `preview-1.png` and `preview-2.png` as screenshots. Put
   `preview-1.png` first: it becomes the product thumbnail.
8. License: **GPL-2.0-or-later**. The source is inside the archive, so the
   GPL source requirement is met even before a public git repository exists.
9. Publish. New products usually go through moderation; the product URL appears
   in your profile.

## After publishing

- Add the product URL to the repository README.
- The public repository is <https://github.com/AhmedAlfahdi/dolphin-colortags>;
  releases live at <https://github.com/AhmedAlfahdi/dolphin-colortags/releases>.
  Use those as the homepage/source links.
- To ship an update, create a new archive with a bumped `project(VERSION ...)`
  in `CMakeLists.txt`, regenerate `dist/`, and add a new release on the product
  page (do not replace the file of an existing release).

## Why there is no upload script

The old OpenDesktop OCS API (`/ocs/v1/content/add`) is gone: the Store replies

```
{"status":"error","statuscode":410,"message":"This API endpoint is no longer supported and has been deprecated."}
```

and the website is additionally protected by Anubis, so automated uploads are
not possible. The web form is the supported path.
