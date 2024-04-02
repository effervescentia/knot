use lang::{types::Type, CanonicalId, NodeId};

#[derive(Clone, Debug, PartialEq)]
pub enum ResolveError {
    NotInferrable(Vec<CanonicalId>),

    NotFound(String, CanonicalId),
}

#[derive(Clone, Debug, PartialEq)]
pub enum SemanticError {
    // NotResolved(ResolveError),

    /* mismatch */
    // UnexpectedShape((ShallowType, NodeId), ExpectedShape),
    // UnexpectedKind((Kind, NodeId), Kind),

    /* enum-related */
    // VariantNotFound((ShallowType, NodeId), String),

    /* module-related */
    // DeclarationNotFound((ShallowType, NodeId), String),

    /* object-related */
    // NotIndexable((ShallowType, NodeId), String),

    /* function-related */
    NotCallable(Type<()>, NodeId),
    // MissingArguments(NodeId, Vec<(ShallowType, NodeId)>),
    // UnexpectedArguments(NodeId, Vec<(ShallowType, NodeId)>),
    // InvalidArguments(
    //     NodeId,
    //     #[allow(clippy::type_complexity)] Vec<((ShallowType, NodeId), (ShallowType, NodeId))>,
    // ),
}
