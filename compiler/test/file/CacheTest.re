open Kore;

module Cache = File.Cache;

let __content = "hello world\n";

let suite =
  "File.Cache"
  >::: [
    "create()"
    >: (
      () =>
        Assert.true_(
          Cache.create("myProject")
          |> String.starts_with(Filename.get_temp_dir_name()),
        )
    ),
    "resolve_path()"
    >: (
      () => {
        let cache = "foo/bar/cache";

        Assert.string(
          Fmt.str("%s/my/path", cache),
          cache |> Cache.resolve_path("my/path"),
        );
      }
    ),
    "file_exists() - file does exist"
    >: (() => Assert.true_(Cache.file_exists(fixture_path, "."))),
    "file_exists() - file does not exist"
    >: (
      () =>
        Assert.false_(
          Cache.file_exists("path/to/nonexistent/file.oops", "."),
        )
    ),
    "open_file()"
    >: (
      () => {
        let open_file = Cache.open_file("read_me.txt", fixture_dir);

        Assert.string(__content, Util.read_channel_to_string(open_file));

        close_in(open_file);
      }
    ),
    "destroy()"
    >: (
      () => {
        let temp_dir = Util.get_temp_dir();
        let parent_dir =
          Filename.concat(temp_dir, Fmt.str("%f", Sys.time()));
        let path =
          Filename.concat(parent_dir, Util.temp_file_name("test", "txt"));

        parent_dir |> FileUtil.mkdir(~parent=true);
        path |> Util.write_to_file(__content);
        Cache.destroy(parent_dir);

        Assert.no_file_exists(path);
        Assert.no_file_exists(parent_dir);
        Assert.file_exists(temp_dir);
      }
    ),
  ];
