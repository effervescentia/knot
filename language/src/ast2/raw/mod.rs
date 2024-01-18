mod augment;
mod collect;

pub struct Node<Value> {
    pub value: Value,
    pub range: super::Range,
}

impl<Value> Node<Value> {
    pub const fn new(value: Value, range: super::Range) -> Self {
        Self { value, range }
    }
}

pub struct Expression(pub Node<super::Expression<Expression, Statement, Component>>);

pub struct Statement(pub Node<super::Statement<Expression>>);

pub struct Component(pub Node<super::Component<Expression, Component>>);

pub struct Parameter(pub Node<super::Parameter<Expression, TypeExpression>>);

pub struct Declaration(pub Node<super::Declaration<Expression, Parameter, Module, TypeExpression>>);

pub struct TypeExpression(pub Node<super::TypeExpression<TypeExpression>>);

pub struct Import(pub Node<super::Import>);

pub struct Module(pub Node<super::Module<Import, Declaration>>);

pub struct Program(Module);
