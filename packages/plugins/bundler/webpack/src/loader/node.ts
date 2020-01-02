import { JSX_PLUGIN, PLATFORM_PLUGIN, STYLE_PLUGIN } from '@knot/compiler';
import { InternalOptions } from '../types';

export default function nodeTransformer(
  result: string,
  { plugins }: InternalOptions
): string {
  return [
    genMainImport(JSX_PLUGIN, plugins.jsx),
    genMainImport(STYLE_PLUGIN, plugins.style),
    genModuleImport(PLATFORM_PLUGIN, plugins.platform),
    result
  ].join('');
}

function genMainImport(name: string, mod: string): string {
  return `import {main as ${name}} from "${mod}";`;
}

function genModuleImport(name: string, mod: string): string {
  return `import * as ${name} from "${mod}";`;
}
