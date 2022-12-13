import { Style } from '@knot/plugin-utils';
import JSS, { Styles } from 'jss';
import jssPreset from 'jss-preset-default';

import * as styleExpressionPlugin from './styleExpressionPlugin';
import * as styleRulePlugin from './styleRulePlugin';

JSS.setup(jssPreset());

const styleSheet = JSS.createStyleSheet<string>({});
let styleSheetAttached = false;
let classID = 0;

export default {
  createStyle(styles: Styles): Style {
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

  classes(...classNames: string[]): string {
    return classNames.join(' ');
  },

  styleExpressionPlugin,
  styleRulePlugin,
};
