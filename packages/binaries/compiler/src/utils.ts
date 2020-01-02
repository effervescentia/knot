import { KNOT_EXTENSION_PATTERN } from './constants';
import { Options } from './types';

export function isKnot(file: string): boolean {
  return KNOT_EXTENSION_PATTERN.test(file);
}

export function resolveLibrary(file: string, options: Options): string | void {
  if (file.startsWith('@knot/')) {
    const [, plugin] = file.split('/');

    if (plugin in options.plugins) {
      return options.plugins[plugin];
    }
  }
}
