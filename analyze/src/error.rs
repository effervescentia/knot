use lang::{ast, types::Kind, CanonicalId};

#[derive(Clone, Debug, PartialEq)]
pub enum Error {
    /* inference */
    NotInferrable(
        // references to the dependencies that blocked inference
        Vec<CanonicalId>,
    ),

    NotFound(
        // name of the expected binding
        String,
    ),

    /* enum-related */
    VariantNotFound(
        // id of the enum declaration
        CanonicalId,
        // name of the expected variant
        String,
    ),

    /* module-related */
    DeclarationNotFound(
        // id of the enum declaration
        CanonicalId,
        // name of the expected declaration
        String,
    ),

    /* object-related */
    NotIndexable(
        // id of the node being indexed
        CanonicalId,
        // name of the expected property
        String,
    ),

    PropertyNotFound(
        // id of the object
        CanonicalId,
        // name of the expected property
        String,
    ),

    DuplicateProperty(
        // name of the duplicate property
        String,
    ),

    NotSpreadable(
        // id of the expression being spread
        CanonicalId,
    ),

    /* function-related */
    NotCallable(
        // id of the node being called
        CanonicalId,
    ),

    /// temporary solution until deeper type inference is implemented
    UntypedParameter,

    DefaultValueRejected(
        // id of the default value
        CanonicalId,
    ),

    UnexpectedArgument(
        // id of the argument
        CanonicalId,
    ),

    MissingArgument(
        // id of the unfulfilled parameter
        CanonicalId,
    ),

    ArgumentRejected(
        // id of the parameter
        CanonicalId,
        // id of the argument
        CanonicalId,
    ),

    /* component-related */
    NotRenderable(
        // id of the expression being rendered
        CanonicalId,
    ),

    InvalidComponent(
        // tag name
        String,
    ),

    ComponentTypo(
        // start tag
        String,
        // end tag
        String,
    ),

    /* mismatch */
    BinaryOperationNotSupported(
        // operation being performed
        ast::BinaryOperator,
        // id of the left-hand side
        CanonicalId,
        // id of the right-hand side
        CanonicalId,
    ),

    UnaryOperationNotSupported(
        // operation being performed
        ast::UnaryOperator,
        // id of the right-hand side
        CanonicalId,
    ),

    UnexpectedKind(
        // id of the node with unmatched kind
        CanonicalId,
        // expected kind
        Kind,
    ),
}
