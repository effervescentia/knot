import JSS, { Styles } from 'jss';
import jssPreset from 'jss-preset-default';

import colors from './colors';
import properties from './properties';
import units from './units';

JSS.setup(jssPreset());

export default {
  ...properties,

  visibility: 'visibility',
  hidden: 'hidden',
  none: 'none',

  // utilities
  num(num: number): number {
    return num;
  },
  str(str: string): string {
    return str;
  },

  ...colors,

  ...units,

  resolve(styles: Styles): Record<any, string> {
    const attached = JSS.createStyleSheet(styles).attach();

    return attached.classes;
  },

  classes(...classNames: string[]): string {
    return classNames.join(' ');
  },
};
