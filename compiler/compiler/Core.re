include Knot.Core;

module Debug = Knot.Debug;

type status =
  | Idle
  | Running
  | Complete
  | Failing(list((compilation_error, string, string)));

type link_descriptor = {
  target: string,
  absolute_path: string,
  relative_path: string,
  pretty_path: string,
};
