include Knot.Core;

module Debug = Knot.Debug;

exception InvalidProgram(string);
exception ModuleNotLoaded(string);

type link_descriptor = {
  target: string,
  absolute_path: string,
  relative_path: string,
  pretty_path: string,
};
