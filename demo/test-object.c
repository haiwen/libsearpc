#include "test-object.h"

G_DEFINE_TYPE(TestObject, test_object, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_INT_A,
  PROP_INT_B,
  PROP_INT_C,
  PROP_STR,
  PROP_BOOL,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void test_object_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  TestObject *self = TEST_OBJECT (object);
  switch (property_id)
  {
  case PROP_INT_A:
    self->a = g_value_get_int(value);
    break;
  case PROP_INT_B:
    self->b = g_value_get_int64(value);
    break;
  case PROP_INT_C:
    self->c = g_value_get_uint(value);
    break;
  case PROP_STR:
    g_free (self->str);
    self->str = g_value_dup_string (value);
    break;
  case PROP_BOOL:
    self->boo = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void test_object_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  TestObject *self = TEST_OBJECT (object);
  switch (property_id)
  {
  case PROP_INT_A:
    g_value_set_int(value, self->a);
    break;
  case PROP_INT_B:
    g_value_set_int64(value, self->b);
    break;
  case PROP_INT_C:
    g_value_set_uint(value, self->c);
    break;
  case PROP_STR:
    g_value_set_string (value, self->str);
    break;
  case PROP_BOOL:
    g_value_set_boolean (value, self->boo);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void test_object_dispose (GObject *object)
{
  g_printf("Instance disposed!\n");
  G_OBJECT_CLASS (test_object_parent_class)->dispose(object);
}

static void test_object_finalize (GObject *object)
{
  g_printf("Instance finalized!\n");
  TestObject *self = TEST_OBJECT (object);
  g_free(self->str);
  G_OBJECT_CLASS (test_object_parent_class)->finalize(object);
}

static void test_object_class_init(TestObjectClass *klass)
{
  g_printf("Class inited!\n");
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = test_object_set_property;
  gobject_class->get_property = test_object_get_property;
  gobject_class->dispose = test_object_dispose;
  gobject_class->finalize = test_object_finalize;

  obj_properties[PROP_INT_A]=g_param_spec_int ("a","","",-200,1000,0,G_PARAM_READWRITE);
  obj_properties[PROP_INT_B]=g_param_spec_int64 ("b","","",0,10000000000,6000000000, G_PARAM_READWRITE);
  obj_properties[PROP_INT_C]=g_param_spec_uint ("c","","",0,100,60, G_PARAM_READWRITE);
  obj_properties[PROP_STR]=g_param_spec_string ("str","","","Hello world!", G_PARAM_READWRITE);
  obj_properties[PROP_BOOL]=g_param_spec_boolean ("boo","","",FALSE,G_PARAM_READWRITE);
  g_object_class_install_properties (gobject_class,N_PROPERTIES,obj_properties);
}

static void test_object_init(TestObject *self)
{
  g_printf("Instance inited!\n");
  self->a=0;
  self->b=6000000000;
  self->c=60;
  self->str=g_strdup("Hello world!");
  self->boo=FALSE;
}
