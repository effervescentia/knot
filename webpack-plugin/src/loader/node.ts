import { GLOBALS } from '../constants';

export default function wrapResult(result: string, { debug, plugins }) {
  if (debug) {
    return `
      var jsxPlugin = require("${plugins.jsx}");
      var stylePlugin = require("${plugins.style}");
      var jsUtils = require("${plugins.utils}");

      module.exports = (${result})({
        "${GLOBALS}": {
          jsx: jsxPlugin,
          style: stylePlugin
        },
        "@knot/jsx": jsxPlugin,
        "@knot/style": stylePlugin
      }, jsUtils);
    `;
  } else {
    return `var jsxPlugin=require("${plugins.jsx}");var stylePlugin=require("${
      plugins.style
    }");var jsUtils=require("${
      plugins.utils
    }");module.exports=(${result})({"${GLOBALS}":{jsx:jsxPlugin,style:stylePlugin},"@knot/jsx":jsxPlugin,"@knot/style":stylePlugin},jsUtils);`;
  }
}
