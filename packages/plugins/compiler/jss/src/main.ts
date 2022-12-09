import JSS, { Styles } from 'jss';
import jssPreset from 'jss-preset-default';

import * as styleExpressionPlugin from './styleExpressionPlugin';
import * as styleRulePlugin from './styleRulePlugin';

JSS.setup(jssPreset());

const styleSheet = JSS.createStyleSheet({});
let styleSheetAttached = false;

export default {
  resolve(styles: Styles): Record<string, string> {
    const attached = JSS.createStyleSheet(styles).attach();

    return attached.classes;
  },

  createStyle(styles: Styles): { getClass: () => string } {
    const rule = styleSheet.addRule(styles);

    return {
      getClass: () => {
        if (!styleSheetAttached) {
          styleSheet.attach();
          styleSheetAttached = true;
        }

        return rule.className;
      },
    };
  },

  classes(...classNames: string[]): string {
    return classNames.join(' ');
  },

  styleExpressionPlugin,
  styleRulePlugin,
};
