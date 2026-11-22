// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/PageColorScript.h"

QString PageColorScript::sampleBackground()
{
    return QStringLiteral(R"js((() => {
  const visibleAlpha = 0.05;
  const hex = (red, green, blue) => "#" + [red, green, blue]
    .map(value => Math.max(0, Math.min(255, Math.round(value))).toString(16).padStart(2, "0"))
    .join("");
  const alphaOf = text => {
    if (text === undefined) return 1;
    return text.endsWith("%") ? parseFloat(text) / 100 : parseFloat(text);
  };
  const parse = text => {
    const rgb = text.match(/^rgba?\(\s*([\d.]+)[,\s]+([\d.]+)[,\s]+([\d.]+)(?:[,\s/]+([\d.]+%?))?\s*\)$/);
    if (rgb) return alphaOf(rgb[4]) < visibleAlpha ? "" : hex(+rgb[1], +rgb[2], +rgb[3]);
    const srgb = text.match(/^color\(srgb\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)(?:\s*\/\s*([\d.]+%?))?\s*\)$/);
    if (srgb) return alphaOf(srgb[4]) < visibleAlpha ? "" : hex(srgb[1] * 255, srgb[2] * 255, srgb[3] * 255);
    return "";
  };
  const probeX = 8;
  let element = document.elementFromPoint(probeX, Math.round(window.innerHeight / 2));
  for (; element; element = element.parentElement) {
    const color = parse(getComputedStyle(element).backgroundColor);
    if (color) return color;
  }
  const meta = document.querySelector('meta[name="theme-color"]');
  return meta && /^#[0-9a-f]{6}$/i.test(meta.content) ? meta.content : "";
})())js");
}
