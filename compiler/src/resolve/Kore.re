include Knot.Kore;

module Cache = File.Cache;

type m_id =
  | Internal(string)
  | External(string);

let print_m_id =
  fun
  | Internal(id) => id |> Print.fmt("@/%s")
  | External(id) => id;

let to_m_id = (s: string): m_id => {
  let file_name =
    String.ends_with(Constants.file_extension, s)
      ? s : s ++ Constants.file_extension;

  String.starts_with(Constants.root_dir, s)
    ? Internal(String.drop_prefix(Constants.root_dir, file_name))
    : External(file_name);
};
