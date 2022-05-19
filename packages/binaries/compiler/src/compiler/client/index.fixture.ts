export const SIMPLE_MAIN = `var $knot = require("@knot/runtime");
var FOO = "foo";
exports.FOO = FOO;
`;

export const COMPLEX_MAIN = `import $knot from "@knot/runtime";
import { main as TIMEOUT } from "./common/constants";
import { main as App } from "./App";
var ABC = 123;
export { ABC };
`;

export const COMPLEX_APP = `import $knot from "@knot/runtime";
var App = $knot.jsx.createTag("div", null, "hello world");
export { App };
export { App as main };
`;

export const COMPLEX_CONSTANTS = `import $knot from "@knot/runtime";
var TIMEOUT = 100;
export { TIMEOUT };
export { TIMEOUT as main };
`;
