use lang::{
    types::{RefKind, ShallowType, Type},
    NodeId,
};

#[derive(Clone, Debug, PartialEq)]
pub enum ExpectedShape {
    Type(Type<usize>),
    Union(Vec<Type<usize>>),
}

#[derive(Clone, Debug, PartialEq)]
pub enum ResolveError {
    NotInferrable(Vec<NodeId>),

    NotFound(String, NodeId),
}

#[derive(Clone, Debug, PartialEq)]
pub enum SemanticError {
    NotResolved(ResolveError),

    /* mismatch */
    UnexpectedShape((ShallowType, NodeId), ExpectedShape),
    UnexpectedKind((RefKind, NodeId), RefKind),

    /* enum-related */
    VariantNotFound((ShallowType, NodeId), String),

    /* module-related */
    DeclarationNotFound((ShallowType, NodeId), String),

    /* object-related */
    NotIndexable((ShallowType, NodeId), String),

    /* function-related */
    NotCallable(ShallowType, NodeId),
    MissingArguments(NodeId, Vec<(ShallowType, NodeId)>),
    UnexpectedArguments(NodeId, Vec<(ShallowType, NodeId)>),
    InvalidArguments(
        NodeId,
        #[allow(clippy::type_complexity)] Vec<((ShallowType, NodeId), (ShallowType, NodeId))>,
    ),
}
