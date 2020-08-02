open Globals;

let _print_file_err = (pretty_path, message) =>
  Printf.sprintf(
    "failed to parse file '%s'\n\n%s",
    pretty_path,
    String.uncapitalize_ascii(message),
  )
  |> print_err;

let _print_syntax_error = (file, print) =>
  fun
  | InvalidCharacter(ch, cursor) => {
      Printf.sprintf(
        "encountered unexpected character '%s' at [%d:%d]",
        Knot.Util.print_uchar(ch),
        fst(cursor),
        snd(cursor),
      )
      |> print;
      CodeFrame.print(file, cursor) |> print_endline;
    }

  | UnclosedCommentBlock(cursor) => {
      Printf.sprintf(
        "no closing tag (*/) found for comment block starting at [%d:%d]",
        fst(cursor),
        snd(cursor),
      )
      |> print;
      CodeFrame.print(file, cursor) |> print_endline;
    }

  | UnclosedString(cursor) => {
      Printf.sprintf(
        "no closing quote (\") found for string starting at [%d:%d]",
        fst(cursor),
        snd(cursor),
      )
      |> print;
      CodeFrame.print(file, cursor) |> print_endline;
    }

  | InvalidDotAccess =>
    "dot operator was not followed by a valid property name" |> print;

let _print_semantic_error =
  fun
  | PropertyDoesNotExist(name) =>
    Printf.sprintf("attempted to access non-existent property '%s'", name)

  | MultipleMainDefinitions => "multiple entities tagged with the 'main' keyword"

  | UsedBeforeDeclaration(name) =>
    Printf.sprintf(
      "entity with the name '%s' was referenced before it was declared",
      name,
    )

  | NameInUse(name) =>
    Printf.sprintf(
      "an entity has already been defined in the current scope with the name '%s'",
      name,
    )

  | ExecutingNonFunction => "attempted to execute an entity that is not callable"

  | OperatorTypeMismatch => "types of operands do not match"

  | UnrenderableReturnType(name) =>
    Printf.sprintf("view '%s' returned a value that cannot be rendered", name)

  | DefaultValueTypeMismatch(name) =>
    Printf.sprintf(
      "the default value does not match the declared type of the property '%s'",
      name,
    )

  | ExportDoesNotExist(module_, name) =>
    Printf.sprintf(
      "the module '%s' does not export the member '%s'",
      module_,
      name,
    )

  | MainExportDoesNotExist(module_) =>
    Printf.sprintf("the module '%s' does not contain a main export", module_)

  | ModuleDoesNotContainExports(module_) =>
    Printf.sprintf("the module '%s' does not contain any exports", module_)

  | UnableToInferType(name) =>
    Printf.sprintf(
      "unable to infer the type of '%s', add a type definition or a default value",
      name,
    )

  | TypeDoesNotExist(name) =>
    Printf.sprintf("found reference to type '%s', which does not exist", name)

  | UnsupportedTypeReference(name) =>
    Printf.sprintf(
      "found reference to type '%s' but expected 'boolean', 'number' or 'string'",
      name,
    )

  | UnsupportedMixinType(name) =>
    Printf.sprintf(
      "found reference to mixin '%s', but it is of the wrong type, expected a 'state' or 'style'",
      name,
    );

let print_compilation_error = (file, pretty_path) => {
  let print_file_err = _print_file_err(pretty_path);

  fun
  | SyntaxError(err) => _print_syntax_error(file, print_file_err, err)

  | SemanticError(err) => _print_semantic_error(err) |> print_file_err

  | ParsingFailed =>
    print_file_err("parser was unable to construct a valid AST")

  | AnalysisFailed(module_) =>
    Printf.sprintf(
      "analysis of module '%s' did not result in a valid AST",
      module_,
    )
    |> print_file_err

  | CircularDependencyDetected =>
    print_file_err("a circular dependency was detected")

  | InvalidImportTarget(name) =>
    Printf.sprintf(
      "module contained an import of the invalid module '%s'",
      name,
    )
    |> print_file_err;
};

let print_execution_error =
  (
    fun
    | MissingRootDirectory => "root directory not provided"

    | InvalidPathFormat(path) =>
      Printf.sprintf("path '%s' is not formatted properly", path)

    | InvalidEntryPoint(path) =>
      Printf.sprintf("unable to load entry point '%s'", path)

    | ModuleDoesNotExist(module_, path) =>
      Printf.sprintf(
        "module '%s' could not be found at path '%s'",
        module_,
        path,
      )

    | EntryPointOutsideBuildContext(path, rootDir) =>
      Printf.sprintf(
        "entry point '%s' is outside of the root build directory '%s'",
        path,
        rootDir,
      )
  )
  % print_err;
