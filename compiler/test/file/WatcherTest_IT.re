open Kore;

module Watcher = File.Watcher;

let __extensions = [".txt"];

let _setup = () => {
  let temp_dir = Util.get_temp_dir();
  let parent_dir = Filename.concat(temp_dir, "foo");
  let path = Filename.concat(parent_dir, "test.txt");

  FileUtil.mkdir(~parent=true, parent_dir);
  Util.write_to_file(path, "hello world");

  (parent_dir, path);
};

let _test_watch = (check, watcher) => {
  Lwt.async(() => {
    watcher
    |> Watcher.(
         watch(
           List.iter(((path, action)) =>
             check((
               /* resolve, */
               path |> String.drop_prefix("/private"),
               action,
             ))
           ),
         )
       )
  });
};

let _flag_to_string =
  Fswatch.Event.(
    fun
    | NoOp => "NoOp"
    | PlatformSpecific => "PlatformSpecific"
    | Created => "Created"
    | Updated => "Updated"
    | Removed => "Removed"
    | Renamed => "Renamed"
    | OwnerModified => "OwnerModified"
    | AttributeModified => "AttributeModified"
    | MovedFrom => "MovedFrom"
    | MovedTo => "MovedTo"
    | IsFile => "IsFile"
    | IsDir => "IsDir"
    | IsSymLink => "IsSymLink"
    | Link => "Link"
    | Overflow => "Overflow"
  );

let suite =
  "File.Watcher"
  >::: [
    "watch() - add file with target extension"
    >~:: (
      (_, ()) => {
        let (parent_dir, path) = _setup();
        let watcher = Watcher.create(parent_dir, __extensions);
        let cancel = Async.await_succcess();
        let (promise, resolve) = Async.wait();

        let new_file = Filename.concat(parent_dir, "added.txt");

        Async.on_tick(() => Util.write_to_file(new_file, "new file"))
        |> ignore;

        watcher
        |> _test_watch(
             fun
             | (name, Add) when name == new_file => {
                 cancel();
                 resolve();
               }
             | _ => (),
           );

        promise;
      }
    ),
    "watch() - add file with ignored extension"
    >~:: (
      (_, ()) => {
        let (parent_dir, path) = _setup();
        let watcher = Watcher.create(parent_dir, __extensions);
        let (promise, resolve) = Async.wait();
        let cancel = Async.on_tick(~ticks=2, resolve);

        Async.on_tick(() =>
          Util.write_to_file(
            Filename.concat(parent_dir, "added.c"),
            "new file",
          )
        )
        |> ignore;

        watcher
        |> _test_watch(
             fun
             | (name, Relocate) when name != path => {
                 cancel();
                 name |> Fmt.str("unexpected file added: %s") |> Assert.fail;
               }
             | _ => (),
           );

        promise;
      }
    ),
    "watch() - rename file"
    >~:: (
      (_, ()) => {
        let (parent_dir, path) = _setup();
        let watcher = Watcher.create(parent_dir, __extensions);
        let (promise, resolve) = Async.wait();
        let cancel = Async.on_tick(~ticks=2, resolve);
        let renamed_file = Filename.concat(parent_dir, "renamed.txt");
        let end_ = () => {
          cancel();
          resolve();
        };
        let source_reported = ref(false);
        let target_reported = ref(false);

        Async.on_tick(() => FileUtil.mv(path, renamed_file)) |> ignore;

        watcher
        |> _test_watch(
             fun
             | (name, Relocate) when name == path =>
               target_reported^ ? end_() : source_reported := true
             | (name, Relocate) when name == renamed_file =>
               source_reported^ ? end_() : target_reported := true
             | _ => (),
           );

        promise;
      }
    ),
    "watch() - update file contents"
    >~:: (
      (_, ()) => {
        let (parent_dir, path) = _setup();
        let watcher = Watcher.create(parent_dir, __extensions);
        let (promise, resolve) = Async.wait();
        let cancel = Async.on_tick(~ticks=2, resolve);

        Async.on_tick(() => Util.append_to_file(path, "more content"))
        |> ignore;

        watcher
        |> _test_watch(
             fun
             | (name, Update) when name == path => {
                 cancel();
                 resolve();
               }
             | _ => (),
           );

        promise;
      }
    ),
    "watch() - remove file"
    >~:: (
      (_, ()) => {
        let (parent_dir, path) = _setup();
        let watcher = Watcher.create(parent_dir, __extensions);
        let (promise, resolve) = Async.wait();
        let cancel = Async.on_tick(~ticks=2, resolve);

        Async.on_tick(() => FileUtil.rm(~recurse=true, [parent_dir]))
        |> ignore;

        watcher
        |> _test_watch(
             fun
             | (name, Remove) => {
                 cancel();
                 resolve();
               }
             | _ => (),
           );

        promise;
      }
    ),
  ];
