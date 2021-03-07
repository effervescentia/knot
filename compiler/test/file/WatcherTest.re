open Kore;

module Watcher = File.Watcher;

let __temp_dir = Filename.get_temp_dir_name();

let unit_tests =
  "File.Watcher"
  >::: [
    "create()"
    >: (
      () => {
        let content = "hello world";
        let parent_dir = Filename.concat(__temp_dir, "foo");
        let path = Filename.concat(parent_dir, "test_create.txt");
        let extensions = [".txt"];

        FileUtil.mkdir(parent_dir);
        Util.write_to_file(path, content);

        let watcher = Watcher.create(parent_dir, extensions);

        watcher.dir |> Assert.string(parent_dir);
        watcher.extensions |> Assert.string_list(extensions);
      }
    ),
  ];

let integration_tests = "File.Watcher" >::: [];
