include Pretty.Formatters;

let relative_path = (cwd: string): t(string) =>
  (ppf, path) =>
    pf(
      ppf,
      "%a %a",
      bold_str,
      path |> Filename.relative_to(cwd),
      grey_str,
      path |> str("(%s)"),
    );
