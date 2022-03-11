open Kore;
open File.FilesystemDriver;

module Watcher = File.Watcher;

let __extensions = [".txt"];
let __init_file = "init.txt";
let __new_file = "new.txt";

let _setup0 = id => {
  let temp_dir = Util.get_temp_dir();
  let parent_dir = Filename.concat(temp_dir, Fmt.str("test_%s", id));
  let path = Filename.concat(parent_dir, __init_file);

  parent_dir |> FileUtil.mkdir(~parent=true);

  (parent_dir, path);
};

let _setup = id => {
  let temp_dir = Util.get_temp_dir();
  let parent_dir = Filename.concat(temp_dir, Fmt.str("test_%s", id));
  let path = Filename.concat(parent_dir, __init_file);

  parent_dir |> FileUtil.mkdir(~parent=true);
  path |> Util.write_to_file("hello world");

  (parent_dir, path);
};

let _setup2 = (id, listener) => {
  let (parent_dir, init_file) = _setup(id);
  let watcher = Watcher.create(parent_dir, __extensions);
  let (promise, resolve) = Async.wait();

  Lwt.async(() =>
    watcher
    |> Watcher.watch(
         List.iter(
           Tuple.map_fst2(String.drop_prefix("/private"))
           % (
             fun
             /* | (path, Watcher.Add) when path == init_file => resolve() */
             | x => listener(x)
           ),
         ),
       )
  );

  (promise, parent_dir, init_file);
};

let _test_watch = (~disabled=ref(false), check, watcher) =>
  Lwt.async(() => {
    watcher
    |> Watcher.watch(
         List.iter(((path, action)) =>
           if (! disabled^) {
             check((path |> String.drop_prefix("/private"), action));
           }
         ),
       )
  });

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

let _setup3 = (id, listener) => {
  let (parent_dir, init_file) = _setup0(id);
  let (promise, resolve) = Async.wait();
  let watcher = Watcher.create(parent_dir, __extensions);
  let is_setup = ref(false);

  Lwt.async(() =>
    watcher
    |> Watcher.watch(
         List.iter(
           fun
           | (path, Add) when ! is_setup^ && path == __init_file => {
               print_endline("SETTTTTUP: " ++ path);
               print_endline("init_file: " ++ init_file);
               is_setup := true;
               resolve();
             }
           | x when is_setup^ => listener(x)
           | _ => print_endline("skipped did not find: " ++ init_file),
         ),
       )
  );

  Async.delay(
    1.0,
    () => {
      parent_dir |> FileUtil.mkdir(~parent=true);
      init_file |> Util.write_to_file("hello world");
    },
  );

  (promise, parent_dir, init_file);
};

let suite =
  "File.Watcher"
  >::: [
    "watch() - setup"
    >~:: (
      (_, ()) => {
        let (done', _, _) = _setup3("watch_setup", ignore);
        let fail = Async.fail_after(10.0);

        [done', fail] |> Lwt.choose;
      }
    ),
    /* "watch() - add file with target extension"
       >~:: (
         (_, ()) => {
           let (done', resolve) = Async.wait();
           let fail = Async.fail_after(10.0);
           let (setup, parent_dir, init_file) =
             _setup3(
               "watch_add",
               fun
               | (path, Add) => resolve(path)
               | _ => (),
             );
           let new_file = Filename.concat(parent_dir, __new_file);

           let action =
             Lwt.bind(setup, () =>
               new_file |> Util.write_to_file("new file") |> Lwt.return
             );

           let result = done' |> Lwt.map(Assert.string(new_file));

           [[result, action] |> Lwt.join, fail] |> Lwt.choose;
         }
       ), */
    /* "watch() - add file with target extension"
       >~:: (
         (_, ()) => {
           let (parent_dir, path) = _setup("watch_add");
           let watcher = Watcher.create(parent_dir, __extensions);
           let (promise, resolve) = Async.await_success();
           let watch_disabled = ref(true);
           let new_file = Filename.concat(parent_dir, __new_file);

           watcher
           |> _test_watch(
                ~disabled=watch_disabled,
                fun
                | (name, Add) when name == new_file => {
                    print_endline("found: " ++ name);
                    resolve();
                  }
                | _ => (),
              );

           Async.on_tick(() => {
             watch_disabled := false;
             new_file |> Util.write_to_file("new file");
           })
           |> ignore;

           promise;
         }
       ),
       "watch() - add file with ignored extension"
       >~:: (
         (_, ()) => {
           let (parent_dir, path) = _setup("watch_ignore_add");
           let watcher = Watcher.create(parent_dir, __extensions);
           let (promise, reject) = Async.await_failure();
           let watch_disabled = ref(true);

           watcher
           |> _test_watch(
                ~disabled=watch_disabled,
                fun
                | (name, Add) when name != path =>
                  name |> Fmt.str("unexpected file added: %s") |> reject

                | _ => (),
              );

           Async.on_tick(() => {
             watch_disabled := false;
             Filename.concat(parent_dir, "added.c")
              |> Util.write_to_file("new file");
           })
           |> ignore;

           promise;
         }
       ),
       "watch() - rename file"
       >~:: (
         (_, ()) => {
           let (parent_dir, path) = _setup("watch_rename");
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
       ), */
    /* "watch() - update file contents"
       >~:: (
         (_, ()) => {
           let (promise, resolve) = Async.await_success(~ticks=4, ());
           let (setup, _, init_file) =
             _setup2(
               "watch_update",
               fun
               | (_, Update) => resolve()
               | _ => (),
             );

           resolve();
           setup;
           /* Lwt.bind(
                setup,
                () => {
                  init_file |> Util.append_to_file("more content");
                  promise;
                },
              ); */
         }
       ), */
    "watch() - add file"
    >~:: (
      (_, ()) => {
        let (done', resolve) = Async.wait();
        let fail = Async.fail_after(10.0);
        let (setup, parent_dir, _) =
          _setup3(
            "watch_add_file",
            fun
            | (path, Add) => resolve(path)
            | _ => (),
          );
        let new_file = Filename.concat(parent_dir, __new_file);

        let action =
          Lwt.bind(setup, () =>
            new_file |> Util.write_to_file("hello world") |> Lwt.return
          );

        let result = done' |> Lwt.map(Assert.string(__new_file));

        [[result, action] |> Lwt.join, fail] |> Lwt.choose;
      }
    ),
    /* "watch() - add folder"
       >~:: (
         (_, ()) => {
           let (done', resolve) = Async.wait();
           let fail = Async.fail_after(10.0);
           let (setup, parent_dir, _) =
             _setup3(
               "watch_add_folder",
               fun
               | (path, Add) => resolve(path)
               | _ => (),
             );
           let new_dir = Filename.concat(parent_dir, "added_dir");

           let action =
             Lwt.bind(setup, () => new_dir |> FileUtil.mkdir |> Lwt.return);

           let result = done' |> Lwt.map(Assert.string(new_dir));

           [[result, action] |> Lwt.join, fail] |> Lwt.choose;
         }
       ), */
    "watch() - remove file"
    >~:: (
      (_, ()) => {
        let (done', resolve) = Async.wait();
        let fail = Async.fail_after(10.0);
        let (setup, _, init_file) =
          _setup3(
            "watch_remove_file",
            fun
            | (path, Remove) => resolve(path)
            | _ => (),
          );

        let action =
          Lwt.bind(setup, () => [init_file] |> FileUtil.rm |> Lwt.return);

        let result = done' |> Lwt.map(Assert.string(__init_file));

        [[result, action] |> Lwt.join, fail] |> Lwt.choose;
      }
    ),
    "watch() - remove folder"
    >~:: (
      (_, ()) => {
        let (done', resolve) = Async.wait();
        let fail = Async.fail_after(10.0);
        let (setup, parent_dir, _) =
          _setup3(
            "watch_remove_folder",
            fun
            | (path, Remove) => resolve(path)
            | _ => (),
          );

        let action =
          Lwt.bind(setup, () =>
            [parent_dir] |> FileUtil.rm(~recurse=true) |> Lwt.return
          );

        let result = done' |> Lwt.map(Assert.string("."));

        [[result, action] |> Lwt.join, fail] |> Lwt.choose;
      }
    ),
  ];
