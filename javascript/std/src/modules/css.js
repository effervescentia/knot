const bindStatic = (key) => Object.assign([function () { }], { toCSS: () => key });
const bindManyStatic = (keys) => Object.fromEntries(keys.map((key) => [key.replaceAll('-', '_'), bindStatic(key)]));

const bindDynamic = (key, encode) => {
  const variant = (...args) => Object.assign([variant, ...args], { toCSS: () => `${key}(${encode ? encode(...args) : args.map(arg => arg.toString()).join(',')})` });
  return variant;
};
const bindManyDynamic = (entries) => Object.fromEntries(entries.map((entry) => Array.isArray(entry) ? [entry[0], bindDynamic(entry[0], entry[1])] : [entry, bindDynamic(entry)]));

const bindUnit = (unit, encode) => {
  const variant = (value) => Object.assign([variant, value], { toCSS: () => `${value.toString()}${unit}` });
  return variant;
};
const bindManyUnits = (entries) => Object.fromEntries(entries.map((unit) => [unit, bindUnit(unit)]));

export const Global = bindManyStatic([
  "initial",
  "inherit",
  "unset",
  "revert",
  "revert-layer",
]);

export const Length = bindManyUnits([
  "cap",
  "ch",
  "em",
  "ex",
  "ic",
  "lh",

  "rcap",
  "rch",
  "rem",
  "rex",
  "ric",
  "rlh",

  "vh",
  "vw",
  "vmax",
  "vmin",
  "vb",
  "vi",

  "cqw",
  "cqh",
  "cqi",
  "cqb",
  "cqmin",
  "cqmax",

  "px",
  "cm",
  "mm",
  "Q",
  "in",
  "pc",
  "pt",
]);

export const AlignContent = bindManyStatic([
  "normal",

  "center",
  "start",
  "end",
  "flex-start",
  "flex-end",

  "baseline",
  "first-baseline",
  "last-baseline",

  "space-between",
  "space-around",
  "space-evenly",
  "stretch",
]);

export const AlignItems = bindManyStatic([
  "normal",
  "stretch",
  "anchor-center",

  "center",
  "start",
  "end",
  "self-start",
  "self-end",
  "flex-start",
  "flex-end",

  "baseline",
  "first-baseline",
  "last-baseline",
]);

export const AlignSelf = bindManyStatic([
  "auto",
  "normal",
  "stretch",
  "anchor-center",

  "center",
  "start",
  "end",
  "self-start",
  "self-end",
  "flex-start",
  "flex-end",

  "baseline",
  "first-baseline",
  "last-baseline",
]);

export const Appearance = bindManyStatic([
  "none",
  "auto",
  "menulist-button",
  "textfield",
]);

export const BackfaceVisibility = bindManyStatic([
  "visible",
  "hidden",
]);

export const BackgroundAttachment = bindManyStatic([
  "scroll",
  "fixed",
  "local",
]);

export const BackgroundClip = bindManyStatic([
  "border-box",
  "padding-box",
  "content-box",
  "text",
]);

export const BackgroundOrigin = bindManyStatic([
  "border-box",
  "padding-box",
  "content-box",
]);

export const BackgroundRepeat = bindManyStatic([
  "repeat",
  "repeat-x",
  "repeat-y",
  "space",
  "round",
  "no-repeat",
]);

export const BackgroundSize = bindManyStatic([
  "cover",
  "contain",
]);

export const BorderStyle = bindManyStatic([
  "dashed",
  "dotted",
  "groove",
]);

export const BoxDecorationBreak = bindManyStatic([
  "slice",
  "clone",
]);

export const BoxSizing = bindManyStatic([
  "content-box",
  "border-box",
]);

export const Break = bindManyStatic([
  "auto",
  "avoid",
  "always",
  "all",

  "avoid-page",
  "page",
  "left",
  "right",
  "recto",
  "verso",

  "avoid-column",
  "column",
  "avoid-region",
  "region",
]);

export const BreakInside = bindManyStatic([
  "auto",
  "avoid",
  "avoid-page",
  "avoid-column",
  "avoid-region",
]);

export const CaptionSide = bindManyStatic([
  "top",
  "bottom",
]);

export const Clear = bindManyStatic([
  "none",
  "left",
  "right",
  "both",
  "inline-start",
  "inline-end",
]);

export const ClipRule = bindManyStatic([
  "nonzero",
  "evenodd",
]);

export const Color = {
  ...bindManyStatic([
    "auto",
    "currentcolor",

    // standard
    "black",
    "silver",
    "gray",
    "white",
    "maroon",
    "red",
    "purple",
    "fuchsia",
    "green",
    "lime",
    "olive",
    "yellow",
    "navy",
    "blue",
    "teal",

    // extended
    "aqua",
    "aliceblue",
    "antiquewhite",
    "aqua",
    "aquamarine",
    "azure",
    "beige",
    "bisque",
    "black",
    "blanchedalmond",
    "blue",
    "blueviolet",
    "brown",
    "burlywood",
    "cadetblue",
    "chartreuse",
    "chocolate",
    "coral",
    "cornflowerblue",
    "cornsilk",
    "crimson",
    "cyan",
    "darkblue",
    "darkcyan",
    "darkgoldenrod",
    "darkgray",
    "darkgreen",
    "darkgrey",
    "darkkhaki",
    "darkmagenta",
    "darkolivegreen",
    "darkorange",
    "darkorchid",
    "darkred",
    "darksalmon",
    "darkseagreen",
    "darkslateblue",
    "darkslategray",
    "darkslategrey",
    "darkturquoise",
    "darkviolet",
    "deeppink",
    "deepskyblue",
    "dimgray",
    "dimgrey",
    "dodgerblue",
    "firebrick",
    "floralwhite",
    "forestgreen",
    "fuchsia",
    "gainsboro",
    "ghostwhite",
    "gold",
    "goldenrod",
    "gray",
    "green",
    "greenyellow",
    "grey",
    "honeydew",
    "hotpink",
    "indianred",
    "indigo",
    "ivory",
    "khaki",
    "lavender",
    "lavenderblush",
    "lawngreen",
    "lemonchiffon",
    "lightblue",
    "lightcoral",
    "lightcyan",
    "lightgoldenrodyellow",
    "lightgray",
    "lightgreen",
    "lightgrey",
    "lightpink",
    "lightsalmon",
    "lightseagreen",
    "lightskyblue",
    "lightslategray",
    "lightslategrey",
    "lightsteelblue",
    "lightyellow",
    "lime",
    "limegreen",
    "linen",
    "magenta",
    "maroon",
    "mediumaquamarine",
    "mediumblue",
    "mediumorchid",
    "mediumpurple",
    "mediumseagreen",
    "mediumslateblue",
    "mediumspringgreen",
    "mediumturquoise",
    "mediumvioletred",
    "midnightblue",
    "mintcream",
    "mistyrose",
    "moccasin",
    "navajowhite",
    "navy",
    "oldlace",
    "olive",
    "olivedrab",
    "orange",
    "orangered",
    "orchid",
    "palegoldenrod",
    "palegreen",
    "paleturquoise",
    "palevioletred",
    "papayawhip",
    "peachpuff",
    "peru",
    "pink",
    "plum",
    "powderblue",
    "purple",
    "rebeccapurple",
    "red",
    "rosybrown",
    "royalblue",
    "saddlebrown",
    "salmon",
    "sandybrown",
    "seagreen",
    "seashell",
    "sienna",
    "silver",
    "skyblue",
    "slateblue",
    "slategray",
    "slategrey",
    "snow",
    "springgreen",
    "steelblue",
    "tan",
    "teal",
    "thistle",
    "tomato",
    "transparent",
    "turquoise",
    "violet",
    "wheat",
    "white",
    "whitesmoke",
    "yellow",
    "yellowgreen",

    // system
    "AccentColor",
    "AccentColorText",
    "ActiveText",
    "ButtonBorder",
    "ButtonFace",
    "ButtonText",
    "Canvas",
    "CanvasText",
    "Field",
    "FieldText",
    "GrayText",
    "Highlight",
    "HighlightText",
    "LinkText",
    "Mark",
    "MarkText",
    "SelectedItem",
    "SelectedItemText",
    "VisitedText",
  ]),
};

export const ColorInterpolation = bindManyStatic([
  "auto",
  "sRGB",
  "linearRGB",
]);

export const ColorScheme = bindManyStatic([
  "normal",
  "light",
  "dark",
  "only-light",
  "only-dark",
  "light-dark",
]);

export const ColumnFill = bindManyStatic([
  "auto",
  "balance",
]);

export const ColumnRuleSet = bindManyStatic([
  "none",
  "hidden",
  "dotted",
  "dashed",
  "solid",
  "double",
  "groove",
  "ridge",
  "inset",
  "outset",
]);

export const ColumnSpan = bindManyStatic([
  "none",
  "all",
]);

export const Contain = bindManyStatic([
  "none",
  "strict",
  "content",
  "size",
  "inline-size",
  "layout",
  "style",
  "paint",
]);

export const ContainerType = bindManyStatic([
  "normal",
  "size",
  "inline-size",
]);

export const ContentVisibility = bindManyStatic([
  "visible",
  "hidden",
  "auto",
]);

export const Cursor = bindManyStatic([
  "auto",
  "default",
  "none",

  "context-menu",
  "help",
  "pointer",
  "progress",
  "wait",

  "cell",
  "crosshair",
  "text",
  "vertical-text",

  "alias",
  "copy",
  "move",
  "no-drop",
  "not-allowed",
  "grab",
  "grabbing",

  "all-scroll",
  "col-resize",
  "row-resize",
  "n-resize",
  "e-resize",
  "s-resize",
  "w-resize",
  "ne-resize",
  "nw-resize",
  "se-resize",
  "sw-resize",
  "ew-resize",
  "ns-resize",
  "nesw-resize",
  "nwse-resize",

  "zoom-in",
  "zoom-out",
]);

export const D = bindManyStatic([
  "none",
]);

export const Direction = bindManyStatic([
  "ltr",
  "rtl",
]);

export const Display = bindManyStatic([
  "block",
  "inline",

  "flow",
  "flow-root",
  "table",
  "flex",
  "grid",
  "ruby",

  "list-item",

  "table-row-group",
  "table-header-group",
  "table-footer-group",
  "table-row",
  "table-cell",
  "table-column-group",
  "table-column",
  "table-caption",
  "ruby-base",
  "ruby-base-container",
  "ruby-text",
  "ruby-text-container",

  "contents",
  "none",

  "inline-block",
  "inline-table",
  "inline-flex",
  "inline-grid",
]);

export const DominantBaseline = bindManyStatic([
  "auto",

  "alphabetic",
  "central",
  "hanging",
  "ideographic",
  "mathematical",
  "middle",
  "text-bottom",
  "text-top",
]);

export const EmptyCells = bindManyStatic([
  "show",
  "hide",
]);

export const FillRule = bindManyStatic([
  "nonzero",
  "evenodd",
]);

export const RowDirection = bindManyStatic([
  "row",
  "row-reverse",
  "column",
  "column-reverse",
]);

export const FlexWrap = bindManyStatic([
  "nowrap",
  "wrap",
  "wrap-reverse",
]);

export const Float = bindManyStatic([
  "block-start",
  "block-end",
  "inline-start",
  "inline-end",
  "left",
  "right",
  "top",
  "bottom",
  "none",
  "footnote",
]);

export const FontKerning = bindManyStatic([
  "auto",
  "normal",
  "none",
]);

export const FontOpticalSizing = bindManyStatic([
  "none",
  "auto",
]);

export const FontPalette = bindManyStatic([
  "normal",
  "light",
  "dark",
]);

export const FontStretch = bindManyStatic([
  "normal",
  "ultra-condensed",
  "extra-condensed",
  "condensed",
  "semi-condensed",
  "semi-expanded",
  "expanded",
  "extra-expanded",
  "ultra-expanded",
]);

export const FontStyle = bindManyStatic([
  "normal",
  "italic",
  "oblique",
]);

export const FontSynthesis = bindManyStatic([
  "none",
  "weight",
  "style",
  "small-caps",
  "position",
]);

export const FontSynthesisStyle = bindManyStatic([
  "auto",
  "none",
]);

export const FontVariantAlternates = bindManyStatic([
  "none",
  "historical-form",
]);

export const FontVariantCaps = bindManyStatic([
  "normal",
  "small-caps",
  "all-small-caps",
  "petite-caps",
  "all-petite-caps",
  "unicase",
  "titling-caps",
]);

export const FontVariantEastAsian = bindManyStatic([
  "normal",
  "ruby",
  "jis78",
  "jis83",
  "jis90",
  "jis04",
  "simplified",
  "traditional",
  "full-width",
  "proportional-width",
]);

export const FontVariantEmoji = bindManyStatic([
  "normal",
  "text",
  "emoji",
  "unicode",
]);

export const FontVariantLigatures = bindManyStatic([
  "normal",
  "none",
  "common-ligatures",
  "no-common-ligatures",
  "discretionary-ligatures",
  "no-discretionary-ligatures",
  "historical-ligatures",
  "no-historical-ligatures",
  "contextual",
  "no-contextual",
]);

export const FontVariantNumeric = bindManyStatic([
  "normal",
  "ordinal",
  "slashed-zero",
  "lining-nums",
  "oldstyle-nums",
  "proportional-nums",
  "tabular-nums",
  "diagonal-fractions",
  "stacked-fractions",
]);

export const FontVariantPosition = bindManyStatic([
  "normal",
  "sub",
  "super",
]);

export const FontWeight = bindManyStatic([
  "normal",
  "bold",
  "lighter",
  "bolder",
]);

export const ForcedColorAdjust = bindManyStatic([
  "auto",
  "none",
]);

export const HangingPunctuation = bindManyStatic([
  "none",
  "first",
  "last",
  "allow-end",
]);

export const Hyphens = bindManyStatic([
  "none",
  "manual",
  "auto",
]);

export const Isolation = bindManyStatic([
  "auto",
  "isolate",
]);

export const MixBlendMode = bindManyStatic([
  "normal",
  "multiply",
  "screen",
  "overlay",
  "darken",
  "lighten",
  "color-dodge",
  "color-burn",
  "hard-light",
  "soft-light",
  "difference",
  "exclusion",
  "hue",
  "saturation",
  "color",
  "luminosity",
  "plus-darker",
  "plus-lighter",
]);

export const ObjectFit = bindManyStatic([
  "contain",
  "cover",
  "fill",
  "none",
  "scale-down",
]);

export const ObjectPosition = bindManyStatic([
  "top",
  "bottom",
  "left",
  "right",
  "center",
]);

export const PaintOrder = bindManyStatic([
  "normal",
  "stroke",
  "markers",
]);

export const PointerEvents = bindManyStatic([
  "auto",
  "none",
]);

export const PrintColorAdjust = bindManyStatic([
  "economy",
  "exact",
]);

export const Resize = bindManyStatic([
  "none",
  "both",
  "horizontal",
  "vertical",
  "block",
  "inline",
]);

export const Rotate = bindManyStatic([
  "none",
]);

export const RubyAlign = bindManyStatic([
  "start",
  "center",
  "space-between",
  "space-around",
]);

export const RubyPosition = bindManyStatic([
  "over",
  "under",
  "alternate",
  "alternate-over",
  "alternate-under",
  "inter-character",
]);

export const TableLayout = bindManyStatic([
  "auto",
  "fixed",
]);

export const TouchAction = bindManyStatic([
  "auto",
  "none",
  "pan-left",
  "pan-right",
  "pan-up",
  "pan-down",
  "pan-x",
  "pan-y",
  "pinch-zoom",
  "manipulation",
]);

export const UserSelect = bindManyStatic([
  "none",
  "auto",
  "text",
  "all",
]);

export const VectorEffect = bindManyStatic([
  "none",
  "non-scaling-stroke",
  "non-scaling-size",
  "non-rotation",
  "fixed-position",
]);

export const VerticalAlign = bindManyStatic([
  "baseline",
  "sub",
  "super",
  "text-top",
  "text-bottom",
  "middle",
  "top",
  "bottom",
]);

export const Visibility = bindManyStatic([
  "visible",
  "hidden",
  "collapse",
]);

export const WhiteSpace = bindManyStatic([
  "normal",
  "nowrap",
  "pre",
  "pre-wrap",
  "pre-line",
  "break-spaces",
]);

export const WhiteSpaceCollapse = bindManyStatic([
  "collapse",
  "preserve",
  "preserve-breaks",
  "preserve-spaces",
  "break-spaces",
]);

export const WillChange = bindManyStatic([
  "auto",
  "scroll-position",
  "contents",
]);

export const WordBreak = bindManyStatic([
  "normal",
  "break-all",
  "keep-all",
  "auto-phrase",
]);

export const WritingMode = bindManyStatic([
  "horizontal-tb",
  "vertical-rl",
  "vertical-lr",
]);
