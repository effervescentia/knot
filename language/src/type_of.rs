use crate::{ast, Node};

pub trait TypeOf {
    fn type_of(&self) -> &ast::typed::Type;
}

impl TypeOf for ast::typed::Type {
    fn type_of(&self) -> &ast::typed::Type {
        self
    }
}

impl TypeOf for ast::typed::Meta {
    fn type_of(&self) -> &ast::typed::Type {
        &self.1
    }
}

impl<Value, Meta> TypeOf for Node<Value, Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.2.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Expression<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Statement<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Component<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::TypeExpression<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Parameter<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Declaration<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Import<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}

impl<Meta> TypeOf for ast::meta::Module<Meta>
where
    Meta: TypeOf,
{
    fn type_of(&self) -> &ast::typed::Type {
        self.0.type_of()
    }
}
