use lang::{ast, types::Kind, CanonicalId};

#[derive(Clone, Debug, PartialEq)]
pub enum ResolveError {
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

    /* function-related */
    NotCallable(
        // id of the node being called
        CanonicalId,
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

    UnexpectedKind(
        // id of the node with unmatched kind
        CanonicalId,
        // expected kind
        Kind,
    ),
}

// #[derive(Clone, Debug, PartialEq)]
// pub enum SemanticError {
//     /* mismatch */
//     // UnexpectedShape((ShallowType, NodeId), ExpectedShape),

//     /* function-related */
//     // MissingArguments(NodeId, Vec<(ShallowType, NodeId)>),
//     // UnexpectedArguments(NodeId, Vec<(ShallowType, NodeId)>),
//     // InvalidArguments(
//     //     NodeId,
//     //     #[allow(clippy::type_complexity)] Vec<((ShallowType, NodeId), (ShallowType, NodeId))>,
//     // ),
// }
