use lang::{types, NodeId};

#[derive(Clone, Debug, PartialEq)]
pub enum ExpectedType {
    Type(types::Type<usize>),
    Union(Vec<types::Type<usize>>),
}

#[derive(Clone, Debug, PartialEq)]
pub enum SemanticError {
    NotInferrable(Vec<NodeId>),

    NotFound(String),

    IllegalValueAccess((types::TypeShape, NodeId), String),
    IllegalTypeAccess((types::TypeShape, NodeId), String),

    ShapeMismatch((types::TypeShape, NodeId), (types::TypeShape, NodeId)),
    UnexpectedShape((types::TypeShape, NodeId), ExpectedType),

    VariantNotFound((types::TypeShape, NodeId), String),

    DeclarationNotFound((types::TypeShape, NodeId), String),

    NotIndexable((types::TypeShape, NodeId), String),

    NotCallable(types::TypeShape, NodeId),
    MissingArguments((types::TypeShape, NodeId), Vec<(types::TypeShape, NodeId)>),
    UnexpectedArguments((types::TypeShape, NodeId), Vec<(types::TypeShape, NodeId)>),
    InvalidArguments(
        (types::TypeShape, NodeId),
        #[allow(clippy::type_complexity)]
        Vec<((types::TypeShape, NodeId), (types::TypeShape, NodeId))>,
    ),
}
