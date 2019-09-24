include Knot.Core;
include Exception;

module Debug = Knot.Debug;

type status =
  | Idle
  | Running
  | Complete;

type link_descriptor = {
  target: string,
  absolute_path: string,
  relative_path: string,
  pretty_path: string,
};
