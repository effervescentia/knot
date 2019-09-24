type compilation_error =
  | LexingFailed
  | ParsingFailed
  | InvalidDotAccess
  | InvalidTypeReference
  | UnclosedCommentBlock
  | InvalidCharacter(Uchar.t, (int, int))
  | MultipleMainDefinitions
  | MissingSidecarScope
  | CircularDependencyDetected
  | InvalidProgram(string)
  | UsedBeforeDeclaration(string)
  | NameInUse(string)
  | UnrenderableReturnType
  | OperatorTypeMismatch
  | ExecutingNonFunction
  | InvalidMixinReference
  | DefaultValueTypeMismatch
  | ImportedModuleDoesNotExist
  | InvalidImport
  | UnanalyzedTypeReference
  | InvalidModule;

exception CompilationError(compilation_error);
exception NotImplemented;

let throw = e => raise(CompilationError(e));

let print_err = (file, error) =>
  ANSITerminal.(
    sprintf(
      [red],
      "[ERROR]: failed to parse file '%s'\n\n%s",
      file,
      String.capitalize_ascii(error),
    )
  )
  |> print_endline;
