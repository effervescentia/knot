include Pretty.Formatters;

let captioned: t((string, string)) =
  (ppf, (main, caption)) =>
    pf(ppf, "%a %a", bold_str, main, grey_str, caption |> str("(%s)"));

let relative_path = (cwd: string): t(string) =>
  (ppf, path) =>
    (path |> Filename.relative_to(cwd), path) |> captioned(ppf);
