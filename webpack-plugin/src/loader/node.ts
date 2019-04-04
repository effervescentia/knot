import { JSX_PLUGIN, STYLE_PLUGIN, UTILS } from '../constants';
import { InternalOptions } from '../types';

export default function wrapResult(
  result: string,
  { plugins }: InternalOptions
): string {
  return [
    genMainImport(JSX_PLUGIN, plugins.jsx),
    genMainImport(STYLE_PLUGIN, plugins.style),
    genModuleImport(UTILS, plugins.utils),
    result
  ].join('');
}

function genMainImport(name: string, mod: string): string {
  return `import {main as ${name}} from '${mod}';`;
}

function genModuleImport(name: string, mod: string): string {
  return `import * as ${name} from '${mod}';`;
}
