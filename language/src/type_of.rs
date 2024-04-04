use crate::{ast, Node, Range};

pub trait TypeOf {
    fn type_of(&self) -> &ast::typed::InnerType;
}

impl TypeOf for ast::typed::InnerType {
    fn type_of(&self) -> &ast::typed::InnerType {
        self
    }
}

impl TypeOf for ast::typed::Type {
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl TypeOf for ast::typed::Meta {
    fn type_of(&self) -> &ast::typed::InnerType {
        self.1.type_of()
    }
}

impl TypeOf for (Range, ast::typed::Meta) {
    fn type_of(&self) -> &ast::typed::InnerType {
        self.1.type_of()
    }
}

impl<Value, Meta> TypeOf for Node<Value, Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.2.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Expression<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Statement<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Component<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::TypeExpression<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Parameter<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Declaration<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Import<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Module<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::InnerType {
        self.0.type_of()
    }
}
