pub struct Node<Value, Type> {
    pub value: Value,
    pub type_: Type,
    range: super::Range,
}

impl<Value, Type> Node<Value, Type> {
    pub fn new(value: Value, type_: Type, range: super::Range) -> Self {
        Self {
            value,
            type_,
            range,
        }
    }
}

pub enum Kind {
    Type,
    Value,
    Mixed,
}

pub enum TypeShape<TypeShape> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,

    Style,
    Component,

    Enumerated(Vec<(String, Vec<TypeShape>)>),
    EnumeratedVariant(Vec<TypeShape>, Box<TypeShape>),
    EnumeratedInstance(Box<TypeShape>),

    View(Vec<TypeShape>),
    Function(Vec<TypeShape>, Box<TypeShape>),
    Module(Vec<(String, Kind, TypeShape)>),
}

pub struct Type(TypeShape<Type>);

pub struct TypeRef<'a>(TypeShape<&'a TypeRef<'a>>);

pub struct Expression<Type>(
    pub Node<super::Expression<Expression<Type>, Statement<Type>, Component<Type>>, Type>,
);

pub struct Statement<Type>(pub Node<super::Statement<Expression<Type>>, Type>);

pub struct Component<Type>(pub Node<super::Component<Expression<Type>, Component<Type>>, Type>);

pub struct Parameter<Type>(
    pub Node<super::Parameter<Expression<Type>, TypeExpression<Type>>, Type>,
);

pub struct Declaration<Type>(
    pub  Node<
        super::Declaration<Expression<Type>, Parameter<Type>, Module<Type>, TypeExpression<Type>>,
        Type,
    >,
);

pub struct TypeExpression<Type>(pub Node<super::TypeExpression<TypeExpression<Type>>, Type>);

pub struct Import<Type>(pub Node<super::Import, Type>);

pub struct Module<Type>(pub Node<super::Module<Import<Type>, Declaration<Type>>, Type>);

pub struct Program(pub Node<super::Program<Module<Type>>, Type>);
