import { Options } from 'nps-utils';

import build from './build';
import cov from './cov';
import doc from './doc';
import fix from './fix';
import test from './test';
import * as Utility from './utility';

export const DEFAULT_OPTIONS: Options = {
  helpStyle: 'scripts'
};

export const DEFAULT_SCRIPTS = {
  ...Utility,

  build,
  fix,
  test,
  cov,
  doc
};

export function extendScripts<T extends object = {}>(
  scripts: T = {} as T
): { options: Options; scripts: typeof DEFAULT_SCRIPTS & T } {
  return {
    options: DEFAULT_OPTIONS,

    scripts: {
      ...DEFAULT_SCRIPTS,

      ...scripts
    }
  };
}

export default extendScripts();
