import JSS, { Styles } from 'jss';
import jssPreset from 'jss-preset-default';

import * as styleExpressionPlugin from './styleExpressionPlugin';
import * as styleRulePlugin from './styleRulePlugin';

JSS.setup(jssPreset());

export default {
  resolve(styles: Styles): Record<any, string> {
    const attached = JSS.createStyleSheet(styles).attach();

    return attached.classes;
  },

  classes(...classNames: string[]): string {
    return classNames.join(' ');
  },

  styleExpressionPlugin,
  styleRulePlugin,
};
