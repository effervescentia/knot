#[derive(Debug, PartialEq)]
pub struct Node<Value, Type> {
    pub value: Value,
    pub type_: Type,
    range: super::Range,
}

impl<Value, Type> Node<Value, Type> {
    pub const fn new(value: Value, type_: Type, range: super::Range) -> Self {
        Self {
            value,
            type_,
            range,
        }
    }
}

#[derive(Debug, PartialEq)]
pub enum Kind {
    Type,
    Value,
    Mixed,
}

#[derive(Debug, PartialEq)]
pub enum TypeShape<TShape> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,

    Style,
    Component,

    Enumerated(Vec<(String, Vec<TShape>)>),
    EnumeratedVariant(Vec<TShape>, Box<TShape>),
    EnumeratedInstance(Box<TShape>),

    View(Vec<TShape>),
    Function(Vec<TShape>, Box<TShape>),
    Module(Vec<(String, Kind, TShape)>),
}

#[derive(Debug, PartialEq)]
pub struct Type(TypeShape<Type>);

#[derive(Debug, PartialEq)]
pub struct TypeRef<'a>(pub TypeShape<&'a TypeRef<'a>>);

#[derive(Debug, PartialEq)]
pub struct Expression<Type>(
    pub Node<super::Expression<Expression<Type>, Statement<Type>, Component<Type>>, Type>,
);

#[derive(Debug, PartialEq)]
pub struct Statement<Type>(pub Node<super::Statement<Expression<Type>>, Type>);

#[derive(Debug, PartialEq)]
pub struct Component<Type>(pub Node<super::Component<Expression<Type>, Component<Type>>, Type>);

#[derive(Debug, PartialEq)]
pub struct Parameter<Type>(
    pub Node<super::Parameter<Expression<Type>, TypeExpression<Type>>, Type>,
);

#[derive(Debug, PartialEq)]
pub struct Declaration<Type>(
    pub  Node<
        super::Declaration<Expression<Type>, Parameter<Type>, Module<Type>, TypeExpression<Type>>,
        Type,
    >,
);

#[derive(Debug, PartialEq)]
pub struct TypeExpression<Type>(pub Node<super::TypeExpression<TypeExpression<Type>>, Type>);

#[derive(Debug, PartialEq)]
pub struct Import<Type>(pub Node<super::Import, Type>);

#[derive(Debug, PartialEq)]
pub struct Module<Type>(pub Node<super::Module<Import<Type>, Declaration<Type>>, Type>);

#[derive(Debug, PartialEq)]
pub struct Program<Type>(pub Module<Type>);
