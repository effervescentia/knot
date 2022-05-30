open Kore;

type t =
  | ModuleFetch(int, ModuleFetch.params_t)
  | ModuleStatus(int, ModuleStatus.params_t)
  | Status(int, Status.params_t)
  | Reset(int, Reset.params_t)
  | ModuleAdd(ModuleAdd.params_t)
  | ModuleUpdate(ModuleUpdate.params_t)
  | ModuleRemove(ModuleRemove.params_t);

let of_module_fetch = (id, params) => ModuleFetch(id, params);
let of_module_status = (id, params) => ModuleStatus(id, params);
let of_status = (id, params) => Status(id, params);
let of_reset = (id, params) => Reset(id, params);
let of_module_add = params => ModuleAdd(params);
let of_module_update = params => ModuleUpdate(params);
let of_module_remove = params => ModuleRemove(params);

let deserialize =
  JSONRPC.Protocol.Event.(
    fun
    | Request(id, key, params) when key == ModuleFetch.method_key =>
      params |> ModuleFetch.deserialize |> of_module_fetch(id) |> Option.some

    | Request(id, key, params) when key == ModuleStatus.method_key =>
      params
      |> ModuleStatus.deserialize
      |> of_module_status(id)
      |> Option.some

    | Request(id, key, params) when key == Status.method_key =>
      params |> Status.deserialize |> of_status(id) |> Option.some

    | Request(id, key, params) when key == Reset.method_key =>
      params |> Reset.deserialize |> of_reset(id) |> Option.some

    | Notification(key, params) when key == ModuleAdd.method_key =>
      params |> ModuleAdd.deserialize |> of_module_add |> Option.some

    | Notification(key, params) when key == ModuleUpdate.method_key =>
      params
      |> ModuleUpdate.deserialize
      |> of_module_update
      |> Option.some

    | Notification(key, params) when key == ModuleRemove.method_key =>
      params
      |> ModuleRemove.deserialize
      |> of_module_remove
      |> Option.some

    | _ => None
  );