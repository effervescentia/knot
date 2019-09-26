export function destructureWithDefault<T>(
  obj: Record<string | number, any>,
  key: string | number,
  defaultVal?: T
): T {
  return key in obj ? obj[key] : defaultVal;
}
