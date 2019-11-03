type syntax_error =
  | /** found an unexpected character */
    InvalidCharacter(Uchar.t, (int, int))
  | /** comment block without closing tag */
    UnclosedCommentBlock((int, int))
  | /** string missing the closing quote */
    UnclosedString
  | /** dot operator was not followed by a valid property name */
    InvalidDotAccess;

type semantic_error =
  | /** referenced a type that does not exist */
    TypeDoesNotExist(string)
  | /** attempted to access non-existent property */
    PropertyDoesNotExist(
      string,
    )
  | /** module contained multiple entities tagged with the main keyword */
    MultipleMainDefinitions
  | /** entity referenced before it was declared */
    UsedBeforeDeclaration(
      string,
    )
  | /** entity declared using a name that is already being used by a different entity in the same scope */
    NameInUse(
      string,
    )
  | /** attempted to execute a non-callable value */
    ExecutingNonFunction
  | /** operation performed on two values of different types */
    OperatorTypeMismatch
  | /** view returned an un-renderable value */
    UnrenderableReturnType(string)
  | /** default value did not match the declared type of the property */
    DefaultValueTypeMismatch(
      string,
    )
  | /** module does not contain any exports */
    ModuleDoesNotContainExports(
      string,
    )
  | /** module does not export the referenced entity */
    ExportDoesNotExist(
      string,
      string,
    )
  | /** modules does not contain a main export */
    MainExportDoesNotExist(
      string,
    )
  | /** could not determine the type of a property as no default value or type definition was provided */
    UnableToInferType(
      string,
    )
  | /** attempted to refer to an un-supported type reference */
    UnsupportedTypeReference(
      string,
    )
  | /** attempted to use an unsupported type as a mixin */
    UnsupportedMixinType(
      string,
    );

type compilation_error =
  | /** failed to parse module */
    ParsingFailed
  | /** failed to analyze module */
    AnalysisFailed(string)
  | /** module imports created a circular dependency */
    CircularDependencyDetected
  | /** module includes an import of an invalid module */
    InvalidImportTarget(
      string,
    )
  | /** a syntax error was thrown while processing module */
    SyntaxError(
      syntax_error,
    )
  | /** a semantic error was thrown while processing module */
    SemanticError(
      semantic_error,
    );

exception CompilationError(compilation_error);
exception NotImplemented;

let throw = e => raise(CompilationError(e));
let throw_syntax = e => throw(SyntaxError(e));
let throw_semantic = e => throw(SemanticError(e));

let print_err = message =>
  ANSITerminal.(sprintf([red], "[ERROR]: %s", message)) |> print_endline;
