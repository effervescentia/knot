// tslint:disable:object-literal-sort-keys
import JSS, { Styles } from 'jss';
import jssPreset from 'jss-preset-default';
import colors from './colors';
import properties from './properties';
import units from './units';

// tslint:disable-next-line:no-expression-statement
JSS.setup(jssPreset());

export const main = {
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

  // tslint:disable-next-line:readonly-array
  classes(...classNames: string[]): string {
    return classNames.join(' ');
  }
};
