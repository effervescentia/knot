import JSS, { Styles } from 'jss';
import jssPreset from 'jss-preset-default';

import * as styleExpressionPlugin from './styleExpressionPlugin';
import * as styleRulePlugin from './styleRulePlugin';

JSS.setup(jssPreset());

const styleSheet = JSS.createStyleSheet<string>({});
let styleSheetAttached = false;
let classID = 0;

export default {
  createStyle(styles: Styles): { getClass: () => string } {
    const id = `k${++classID}`;

    styleSheet.addRule(id, styles);

    return {
      getClass: () => {
        if (!styleSheetAttached) {
          styleSheet.attach();
          styleSheetAttached = true;
        }

        return styleSheet.classes[id];
      },
    };
  },

  // bindStyle(styles: Styles): JSX.Element {},

  classes(...classNames: string[]): string {
    return classNames.join(' ');
  },

  styleExpressionPlugin,
  styleRulePlugin,
};
