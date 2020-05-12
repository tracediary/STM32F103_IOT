#include "cJSON_Process.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"
/*******************************************************************
 *              define var
 *******************************************************************/

cJSON* cJSON_Data_Init(void)
{
	cJSON* cJSON_Root = NULL;    //json  root

	char* p = NULL;
	cJSON_Root = cJSON_CreateObject(); /*create obj*/
	if (NULL == cJSON_Root)
	{
		return NULL;
	}
	//TODO 添加键值对元素
	cJSON_AddStringToObject(cJSON_Root, NAME, DEFAULT_NAME); /**add key-value*/
	cJSON_AddNumberToObject(cJSON_Root, TEMP_NUM, DEFAULT_TEMP_NUM);
	cJSON_AddNumberToObject(cJSON_Root, HUM_NUM, DEFAULT_HUM_NUM);

#if 1
	p = cJSON_Print(cJSON_Root); /*p point JSON string*/
	
	printf("get the json: %s\n",p);

	vPortFree(p);
	p = NULL;
#endif
	return cJSON_Root;

}
uint8_t cJSON_Update(const cJSON * const object, const char * const string, void *d)
{
	cJSON* node = NULL;
	node = cJSON_GetObjectItem(object, string);
	if (node == NULL)
		return NULL;
	if (cJSON_IsBool(node))
	{
		int *b = (int*) d;
		cJSON_GetObjectItem(object, string)->type = *b ? cJSON_True : cJSON_False;
		return 1;
	}
	else if (cJSON_IsString(node))
	{
		cJSON_GetObjectItem(object, string)->valuestring = (char*) d;
		return 1;
	}
	else if (cJSON_IsNumber(node))
	{
		double *num = (double*) d;
//    cJSON_GetObjectItem(object,string)->valueint = (double)*num;
		cJSON_GetObjectItem(object, string)->valuedouble = (double) *num;
//    char* p = cJSON_Print(object);
		return 1;
	}
	else
		return 1;
}

void Proscess(void* data)
{
	printf("start to process json.");
	cJSON *root, *json_name, *json_temp_num, *json_hum_num;
	root = cJSON_Parse((char*) data); //parse json string

	//TODO 改变元素
	json_name = cJSON_GetObjectItem(root, NAME);  //get key-value
	json_temp_num = cJSON_GetObjectItem(root, TEMP_NUM);
	json_hum_num = cJSON_GetObjectItem(root, HUM_NUM);

	printf("name:%s\n temp_num:%f\n hum_num:%f\n", json_name->valuestring, json_temp_num->valuedouble,
			json_hum_num->valuedouble);

	cJSON_Delete(root);
}

