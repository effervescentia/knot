import animationProperties from './animation';
import backgroundProperties from './background';
import borderProperties from './border';
import fontProperties from './font';
import gridProperties from './grid';
import insetProperties from './inset';
import marginProperties from './margin';
import maskProperties from './mask';
import outlineProperties from './outline';
import paddingProperties from './padding';
import scrollProperties from './scroll';
import textProperties from './text';
import transformProperties from './transform';
import transitionProperties from './transition';

export default {
  alignContent: 'alignContent',
  alignItems: 'alignItems',
  alignSelf: 'alignSelf',

  all: 'all',

  ...animationProperties,

  ...backgroundProperties,

  blockSize: 'blockSize',

  ...borderProperties,

  bottom: 'bottom',

  boxDecorationBreak: 'boxDecorationBreak',
  boxShadow: 'boxShadow',
  boxSizing: 'boxSizing',

  breakAfter: 'breakAfter',
  breakBefore: 'breakBefore',
  breakInside: 'breakInside',

  captionSide: 'captionSide',
  caretColor: 'caretColor',
  clear: 'clear',

  clip: 'clip',
  clipPath: 'clipPath',

  color: 'color',
  colorAdjust: 'colorAdjust',

  columnCount: 'columnCount',
  columnFill: 'columnFill',
  columnGap: 'columnGap',
  columnRule: 'columnRule',
  columnRuleColor: 'columnRuleColor',
  columnRuleStyle: 'columnRuleStyle',
  columnRuleWidth: 'columnRuleWidth',
  columns: 'columns',

  content: 'content',

  counterIncrement: 'counterIncrement',
  counterReset: 'counterReset',

  cursor: 'cursor',
  direction: 'direction',
  display: 'display',
  emptyCells: 'emptyCells',
  filter: 'filter',

  flex: 'flex',
  flexBasis: 'flexBasis',
  flexDirection: 'flexDirection',
  flexFlow: 'flexFlow',
  flexGrow: 'flexGrow',
  flexShrink: 'flexShrink',
  flexWrap: 'flexWrap',

  float: 'float',

  ...fontProperties,

  gap: 'gap',
  grad: 'grad',

  ...gridProperties,

  hangingPunctuation: 'hangingPunctuation',
  height: 'height',
  hyphens: 'hyphens',

  imageOrientation: 'imageOrientation',
  imageRendering: 'imageRendering',

  inherit: 'inherit',
  initial: 'initial',
  inlineSize: 'inlineSize',

  ...insetProperties,

  isolation: 'isolation',

  justifyContent: 'justifyContent',
  justifyItems: 'justifyItems',
  justifySelf: 'justifySelf',

  left: 'left',
  letterSpacing: 'letterSpacing',

  lineBreak: 'lineBreak',
  lineHeight: 'lineHeight',

  listStyle: 'listStyle',
  listStyleImage: 'listStyleImage',
  listStylePosition: 'listStylePosition',
  listStyleType: 'listStyleType',

  ...marginProperties,

  ...maskProperties,

  maxHeight: 'maxHeight',
  maxWidth: 'maxWidth',
  maxZoom: 'maxZoom',
  minBlockSize: 'minBlockSize',
  minHeight: 'minHeight',
  minInlineSize: 'minInlineSize',
  minWidth: 'minWidth',
  minZoom: 'minZoom',
  mixBlendMode: 'mixBlendMode',
  negative: 'negative',

  objectFit: 'objectFit',
  objectPosition: 'objectPosition',

  opacity: 'opacity',
  order: 'order',
  orientation: 'orientation',
  orphans: 'orphans',

  ...outlineProperties,

  overflow: 'overflow',
  overflowWrap: 'overflowWrap',
  overflowX: 'overflowX',
  overflowY: 'overflowY',

  pad: 'pad',

  ...paddingProperties,

  pageBreakAfter: 'pageBreakAfter',
  pageBreakAfterBefore: 'pageBreakAfterBefore',
  pageBreakAfterInside: 'pageBreakAfterInside',

  perspective: 'perspective',
  perspectiveOrigin: 'perspectiveOrigin',

  placeContent: 'placeContent',
  placeItems: 'placeItems',
  placeSelf: 'placeSelf',

  pointerEvents: 'pointerEvents',

  position: 'position',
  static: 'static',
  relative: 'relative',
  absolute: 'absolute',
  sticky: 'sticky',
  fixed: 'fixed',

  prefix: 'prefix',
  quotes: 'quotes',
  rad: 'rad',
  range: 'range',
  resize: 'resize',
  revert: 'revert',
  right: 'right',
  rotate: 'rotate',
  rowGap: 'rowGap',
  scale: 'scale',

  ...scrollProperties,

  shapeImageThreshold: 'shapeImageThreshold',
  shapeMargin: 'shapeMargin',
  shapeOutside: 'shapeOutside',

  src: 'src',
  suffix: 'suffix',
  symbols: 'symbols',
  system: 'system',
  tabSize: 'tabSize',
  tableLayout: 'tableLayout',

  ...textProperties,

  top: 'top',
  touchAction: 'touchAction',

  ...transformProperties,

  ...transitionProperties,

  translate: 'translate',
  turn: 'turn',
  unicodeBidi: 'unicodeBidi',
  unicodeRange: 'unicodeRange',
  unset: 'unset',
  userZoom: 'userZoom',
  verticalAlign: 'verticalAlign',
  vmax: 'vmax',
  vmin: 'vmin',
  whiteSpace: 'whiteSpace',
  widows: 'widows',
  width: 'width',
  willChange: 'willChange',

  wordBreak: 'wordBreak',
  wordSpacing: 'wordSpacing',
  wordWrap: 'wordWrap',

  writingMode: 'writingMode',
  zIndex: 'zIndex',

  // @experimental
  backfaceVisibility: 'backfaceVisibility'
};
