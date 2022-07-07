export const isMultisetEqual = <T>(lhs: T[], rhs: T[]) => {
  if (lhs.length !== rhs.length) {
    return false;
  }

  const remaining = [...rhs];

  return lhs.every((value) => {
    const foundIndex = remaining.indexOf(value);

    if (foundIndex === -1) {
      return false;
    }

    remaining.splice(foundIndex, 1);

    return true;
  });
};
