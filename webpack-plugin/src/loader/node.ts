import { GLOBALS, MODULES, UTILS } from '../constants';
import { InternalOptions } from '../types';

export default function wrapResult(
  result: string,
  { plugins }: InternalOptions
): string {
  return `import {main as $$_jsxPlugin} from '${
    plugins.jsx
  }';import {main as $$_stylePlugin} from '${
    plugins.style
  }';import * as ${UTILS} from '${
    plugins.utils
  }';var ${MODULES}={'${GLOBALS}':{jsx:$$_jsxPlugin,style:$$_stylePlugin},'@knot/jsx':$$_jsxPlugin,'@knot/style':$$_stylePlugin};${result}`;
}
