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
    ? Internal(String.sub(file_name, 2, String.length(file_name) - 2))
    : External(file_name);
};
