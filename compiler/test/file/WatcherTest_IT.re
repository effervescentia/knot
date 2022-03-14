open Kore;
open File.FilesystemDriver;

module Watcher = File.Watcher;

let __init_file = "init.txt";
let __new_file = "new.txt";
let __new_dir = "new_dir";
let __renamed_file = "renamed.txt";

let _setup = (test_id, listener) => {
  let temp_dir = Util.get_temp_dir();
  let root_dir = Filename.concat(temp_dir, "scratch");
  let test_dir = Filename.concat(root_dir, test_id);
  let init_file = Filename.concat(test_dir, __init_file);

  test_dir |> FileUtil.mkdir(~parent=true);

  let (promise, resolve) = Async.wait();
  let watcher = Watcher.create(root_dir);
  let is_setup = ref(false);

  Lwt.async(() =>
    watcher
    |> Watcher.watch(
         List.iter(
           fun
           | (path, Add)
               when
                 ! is_setup^ && path == Filename.concat(test_id, __init_file) => {
               is_setup := true;
               resolve();
             }
           | x when is_setup^ => listener(x)
           | _ => (),
         ),
       )
  );

  Async.delay(1.0, () => init_file |> Util.write_to_file("hello world"));

  (promise, test_dir);
};

let test_darwin = () => {
  let test_id = "watch_filesystem_manipulation";
  let fail = Async.fail_after(30.0);
  let (add_file, done_add_file) = Async.wait();
  let (add_dir, done_add_dir) = Async.wait();
  let (add_nested_file, done_add_nested_file) = Async.wait();
  let (remove_file, done_remove_file) = Async.wait();
  let (remove_dir, done_remove_dir) = Async.wait();
  let (remove_nested_file, done_remove_nested_file) = Async.wait();
  let (update_file, done_update_file) = Async.wait();
  let (move_file_add, done_move_file_add) = Async.wait();
  let (move_file_remove, done_move_file_remove) = Async.wait();

  let (setup, test_dir) =
    _setup(
      test_id,
      fun
      | (path, Add) when path == Filename.concat(test_id, __new_file) =>
        done_add_file()

      | (path, Add) when path == Filename.concat(test_id, __new_dir) =>
        done_add_dir()

      | (path, Add)
          when path == Filename.join([test_id, __new_dir, __new_file]) =>
        done_add_nested_file()

      | (path, Add) when path == Filename.concat(test_id, __renamed_file) =>
        done_move_file_add()

      | (path, Remove) when path == Filename.concat(test_id, __new_file) =>
        done_remove_file()

      | (path, Remove) when path == Filename.concat(test_id, __new_dir) =>
        done_remove_dir()

      | (path, Remove)
          when path == Filename.join([test_id, __new_dir, __new_file]) =>
        done_remove_nested_file()

      | (path, Remove) when path == Filename.concat(test_id, __init_file) =>
        done_move_file_remove()

      | (path, Update) when path == Filename.concat(test_id, __new_file) =>
        done_update_file()

      | _ => (),
    );

  let new_file = Filename.concat(test_dir, __new_file);
  let new_dir = Filename.concat(test_dir, __new_dir);
  let new_nested_file = Filename.concat(new_dir, __new_file);
  let renamed_file = Filename.concat(test_dir, __renamed_file);

  let actions =
    Lwt.(
      setup
      >>= (
        () => {
          new_file |> Util.write_to_file("hello world");
          add_file;
        }
      )
      >>= (
        () => {
          new_file |> Util.append_to_file("more content");
          update_file;
        }
      )
      >>= (
        () => {
          [new_file] |> FileUtil.rm;
          remove_file;
        }
      )
      >>= (
        () => {
          FileUtil.mkdir(new_dir);
          add_dir;
        }
      )
      >>= (
        () => {
          new_nested_file |> Util.write_to_file("hello world");
          add_nested_file;
        }
      )
      >>= (
        () => {
          [new_dir] |> FileUtil.rm(~recurse=true);
          [remove_dir, remove_nested_file] |> Lwt.join;
        }
      )
      >>= (
        () => {
          FileUtil.mv(Filename.concat(test_dir, __init_file), renamed_file);
          [move_file_add, move_file_remove] |> Lwt.join;
        }
      )
    );

  [actions, fail] |> Lwt.choose;
};

let suite =
  "File.Watcher"
  >::: [
    "watch() - filesystem manipulation"
    >~:: (
      _ =>
        switch (File.Platform.get()) {
        | Darwin => test_darwin
        | _ => Lwt.return
        }
    ),
  ];
