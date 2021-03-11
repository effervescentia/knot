include Knot.Kore;

module Cache = File.Cache;

type m_id =
  | Internal(string)
  | External(string);

let print_m_id =
  fun
  | Internal(id) => Constants.root_dir ++ id
  | External(id) => id;

let to_m_id = (s: string): m_id =>
  String.starts_with(Constants.root_dir, s)
    ? Internal(String.drop_prefix(Constants.root_dir, s)) : External(s);
