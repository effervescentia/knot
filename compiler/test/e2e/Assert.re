open Knot.Kore;

include TestLibrary.Assert;

let rec directory = (expected: string, actual: string) => {
  let expected_files = FileUtil.ls(expected);
  let expected_count = List.length(expected_files);
  let actual_files = FileUtil.ls(actual);
  let actual_count = List.length(actual_files);

  if (expected_count != actual_count) {
    Fmt.str(
      "expected directory %s to contain %d file(s) but found %d instead",
      actual,
      expected_count,
      actual_count,
    )
    |> fail;
  };

  expected_files
  |> List.iter(expected_file => {
       let relative_file = expected_file |> Filename.relative_to(expected);
       let actual_file = relative_file |> Filename.concat(actual);

       if (actual_files |> List.mem(actual_file)) {
         if (Sys.is_directory(expected_file)) {
           if (Sys.is_directory(actual_file)) {
             directory(expected_file, actual_file);
           } else {
             Fmt.str("expected file %s to be a directory", actual_file)
             |> fail;
           };
         } else if (FileUtil.cmp(expected_file, actual_file) != None) {
           Fmt.str(
             "expected file %s to have the same contents as file %s",
             actual_file,
             expected_file,
           )
           |> fail;
         };
       } else {
         Fmt.str(
           "expected directory %s to contain file %s",
           actual,
           relative_file,
         )
         |> fail;
       };
     });
};
