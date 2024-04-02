use lang::CanonicalId;

#[derive(Clone, Debug, PartialEq)]
pub enum ResolveError {
    /* inference */
    NotInferrable(
        // references to the dependencies that blocked inference
        Vec<CanonicalId>,
    ),
    NotFound(String, CanonicalId),

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
        // id of the target node
        CanonicalId,
        // name of the expected property
        String,
    ),

    /* function-related */
    NotCallable(
        // id of the target node
        CanonicalId,
    ),
}

// #[derive(Clone, Debug, PartialEq)]
// pub enum SemanticError {
//     // NotResolved(ResolveError),

//     /* mismatch */
//     // UnexpectedShape((ShallowType, NodeId), ExpectedShape),
//     // UnexpectedKind((Kind, NodeId), Kind),

//     /* function-related */
//     // MissingArguments(NodeId, Vec<(ShallowType, NodeId)>),
//     // UnexpectedArguments(NodeId, Vec<(ShallowType, NodeId)>),
//     // InvalidArguments(
//     //     NodeId,
//     //     #[allow(clippy::type_complexity)] Vec<((ShallowType, NodeId), (ShallowType, NodeId))>,
//     // ),
// }
