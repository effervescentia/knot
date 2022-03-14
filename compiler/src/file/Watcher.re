open Kore;
open Fswatch;
open Lwt;

/**
 watches a directory structure and reacts to files changing
 */
type t = {
  dir: string,
  driver: FilesystemDriver.t,
};

type dispatch_t = list((string, FilesystemDriver.action_t)) => unit;

let _flag_to_string: Fswatch.Event.flag => string =
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

let _drop_platform_prefix =
  switch (Platform.get()) {
  | Darwin => String.drop_prefix("/private")
  | _ => Fun.id
  };

let rec _listen = (dispatch: dispatch_t, watcher: t, msgBox) =>
  Lwt_mvar.take(msgBox)
  >>= (
    events => {
      events
      |> Array.to_list
      |> List.filter_map(
           Event.(
             event => {
               let path = _drop_platform_prefix(event.path);
               let path = path |> Filename.relative_to(watcher.dir);
               let path = path == "" ? "." : path;

               print_endline("file found: " ++ path);
               print_endline(
                 "flags: "
                 ++ (
                   event.flags
                   |> Array.map(_flag_to_string)
                   |> Array.fold_left((++), "")
                 ),
               );

               watcher.driver(event.path, event.flags)
               |> Option.map(action => (path, action));
             }
           ),
         )
      |> dispatch;

      flush(stdout);
      _listen(dispatch, watcher, msgBox);
    }
  );

/* static */

let create = (dir: string) => {
  switch (init_library()) {
  | Status.FSW_OK => ()
  | status => raise(WatchFailed(Status.t_to_string(status)))
  };

  {dir, driver: FilesystemDriver.create()};
};

/* methods */

/**
 start watching a folder for file system events
 */
let watch = (dispatch: dispatch_t, watcher: t) => {
  let (handle, msgBox) = Fswatch_lwt.init_session(Monitor.System_default);

  add_path(handle, watcher.dir);

  Lwt.async(Fswatch_lwt.start_monitor(handle));

  _listen(dispatch, watcher, msgBox);
};
