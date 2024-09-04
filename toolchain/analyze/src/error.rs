use lang::{
    ast,
    types::{self, Kind},
    CanonicalId,
};

#[derive(Clone, Debug, Eq, PartialEq)]
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
        // names of the valid variants
        Vec<String>,
        // name of the expected variant
        String,
    ),

    /* module-related */
    DeclarationNotFound(
        // id of the enum declaration
        CanonicalId,
        // names of the valid declarations
        Vec<String>,
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
        // names of the valid properties
        Vec<String>,
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

    /* callable-related */
    /// temporary solution until deeper type inference is implemented
    UntypedParameter(String),

    DefaultValueRejected(
        (
            // id of the type definition
            CanonicalId,
            // the type of the parameter
            types::Shape,
        ),
        (
            // id of the default value
            CanonicalId,
            // the type of the default value
            types::Shape,
        ),
    ),

    /* function-related */
    NotCallable(
        // id of the node being called
        CanonicalId,
    ),

    UnexpectedArgument(
        // id of the argument
        CanonicalId,
        // number of arguments expected
        usize,
    ),

    MissingArgument(
        // id of the unfulfilled parameter
        CanonicalId,
        // the type of the parameter
        types::Shape,
    ),

    ArgumentRejected(
        (
            // id of the parameter
            CanonicalId,
            // the type of the parameter
            types::Shape,
        ),
        (
            // id of the argument
            CanonicalId,
            // the type of the argument
            types::Shape,
        ),
    ),

    /* component-related */
    NotRenderable(
        // id of the expression being rendered
        CanonicalId,
    ),

    InvalidComponent(
        // tag name
        String,
        // the type of the value
        types::Shape,
    ),

    ComponentTypo(
        // start tag
        String,
        // end tag
        String,
    ),

    InvalidAttributes(
        // id of the attributes type
        CanonicalId,
    ),

    UnexpectedAttribute(
        // id of the unexpected attribute
        CanonicalId,
        // name of the attribute
        String,
    ),

    MissingAttribute(
        // id of the unfulfilled attribute
        CanonicalId,
        // name of the attribute
        String,
        // the type of the attribute
        types::Shape,
    ),

    AttributeRejected(
        (
            // id of the parameter
            CanonicalId,
            // name of the attribute
            String,
            // the type of the parameter
            types::Shape,
        ),
        (
            // id of the argument
            CanonicalId,
            // the type of the argument
            types::Shape,
        ),
    ),

    /* style-related */
    StyleRuleNotFound(
        // name of the rule
        String,
    ),

    StyleRuleRejected(
        // name of the attribute
        String,
        // the expected type of the attribute
        types::Shape,
        // the actual type of the attribute
        types::Shape,
    ),

    /* mismatch */
    BinaryOperationNotSupported(
        // operation being performed
        ast::BinaryOperator,
        (
            // id of the left-hand side
            CanonicalId,
            // the type of the left-hand side
            Option<types::Shape>,
        ),
        (
            // id of the right-hand side
            CanonicalId,
            // the type of the right-hand side
            Option<types::Shape>,
        ),
    ),

    UnaryOperationNotSupported(
        // operation being performed
        ast::UnaryOperator,
        (
            // id of the right-hand side
            CanonicalId,
            // the type of the right-hand side
            Option<types::Shape>,
        ),
    ),

    UnexpectedKind(
        // id of the node with unmatched kind
        CanonicalId,
        // expected kind
        Kind,
    ),
}
