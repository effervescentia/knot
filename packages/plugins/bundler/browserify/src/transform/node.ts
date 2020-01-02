import {
  JSX_PLUGIN,
  Options,
  PLATFORM_PLUGIN,
  STYLE_PLUGIN
} from '@knot/compiler';

export default function nodeTransformer(
  result: string,
  { plugins }: Options
): string {
  return [
    genMainImport(JSX_PLUGIN, plugins.jsx),
    genMainImport(STYLE_PLUGIN, plugins.style),
    genModuleImport(PLATFORM_PLUGIN, plugins.platform),
    result
  ].join('');
}

function genMainImport(name: string, mod: string): string {
  return `var ${name} = require("${mod}").main;`;
}

function genModuleImport(name: string, mod: string): string {
  return `var ${name} = require("${mod}");`;
}
