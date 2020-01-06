import { destructureWithDefault } from './utils';

const PARAMETER_MAP = '$$KNOT_PARAM_MAP$$';

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
  name: string,
  defaultVal: T
): T | undefined {
  if (argumentsObj.length === 0) {
    return undefined;
  }

  const hasDefaults = argumentsObj.length > 3;
  const hasParamMap =
    argumentsObj.length === 1 &&
    typeof argumentsObj[0] === 'object' &&
    PARAMETER_MAP in argumentsObj[0];
  const [key, args] = hasParamMap
    ? [name, argumentsObj[0]]
    : [index, argumentsObj];

  if (hasDefaults) {
    return destructureWithDefault(args, key, defaultVal);
  } else {
    return args[key];
  }
}

export function style(
  stylePlugin: StylePlugin,
  styles: Record<string, any>
): Record<string, string> {
  const jss = {};

  Object.keys(styles).forEach(key => {
    if (key[0] !== '.') {
      return;
    }

    jss[key.slice(1)] = styles[key];
  });

  return stylePlugin.resolve(jss);
}
