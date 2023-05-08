import { destructureWithDefault } from './utils';

interface StylePlugin {
  readonly resolve: (jss: Record<string, any>) => Record<string, string>;
}

export function prop<T>(
  props: Record<string, any>,
  name: string,
  defaultVal?: T
): T {
  return destructureWithDefault<T>(props, name, defaultVal);
}

export function arg<T>(
  argumentsObj: readonly any[],
  index: number,
  defaultVal: T
): T | undefined {
  if (argumentsObj.length - 1 < index) {
    return defaultVal;
  }

  return argumentsObj[index];
}

export function style(
  stylePlugin: StylePlugin,
  styles: Record<string, any>
): Record<string, string> {
  const jss = {};
  const global = {};

  Object.keys(styles).forEach((key) => {
    if (key[0] === '.') {
      jss[key.slice(1)] = styles[key];
    } else {
      global[key] = styles[key];
    }
  });

  if (Object.keys(global).length) {
    jss['@global'] = global;
  }

  return stylePlugin.resolve(jss);
}
