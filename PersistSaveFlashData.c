

uint32_t  CalcCrc(uint32_t *pdwBuf, uint16_t  cLen)
{
	uint32_t dwValue = 0;
	uint16_t  i;

	for(i=0; i<cLen; i++)
	{
	      dwValue += pdwBuf[i];
	}

	return dwValue;
}

uint8_t SaveCodeInformation(E_FLASH_INFO InfoNum,unsigned int DA)	 
{
   uint16_t i;
   uint32_t BackupData[BackupDataMax];
   //uint32_t *BackupData = (uint32_t *)DataFlashBuffer;
   uint32_t l_temp;
   uint32_t dwValue;


   SYS_UnlockReg();	 
   FMC_Open();
   FMC_ENABLE_AP_UPDATE();
    
   for(i=0;i<BackupDataMax;i++)
	BackupData[i] = FMC_Read(CODEINFOBASE+i*4);   
 


   if ( FMC_Erase(CODEINFOBASE)==-1 )
   {
       FMC_Close();
   	   SYS_LockReg();
       return FALSE;
	}

  
   BackupData[InfoNum] = DA;
		

   for(i=0;i<BackupDataMax;i++)
   {
   	  FMC_Write(CODEINFOBASE+i*4,BackupData[i]);
   }

   FMC_Write(CODEINFOBASE+(CHECK_SUM_ADDRESS-1)*4, CalcCrc(BackupData,BackupDataMax));
     


    for(i=0;i<BackupDataMax;i++)
   {
   	  l_temp = FMC_Read(CODEINFOBASE+i*4);
	
	if ( l_temp!= BackupData[i] )
	{
		FMC_Close();
		SYS_LockReg();
		return FALSE;
	}  
    }  
   

   for(i=0;i<BackupDataMax;i++)
   {	 
   	  BackupData[i] = FMC_Read(TESTPAGEBASE+i*4);
   }       

   if ( FMC_Erase(TESTPAGEBASE)==-1 )
   {
       FMC_Close();
   	   SYS_LockReg();
       return FALSE;
  }				   

 
  BackupData[InfoNum] = Encrypt(DA);

   for(i=0;i<BackupDataMax;i++)
   {
   	  FMC_Write(TESTPAGEBASE+i*4,BackupData[i]);
   } 
  
   FMC_Write(TESTPAGEBASE+(CHECK_SUM_ADDRESS-1)*4, CalcCrc(BackupData,BackupDataMax));  
  
   
   FMC_Close();
   SYS_LockReg();

   return TRUE;
}

unsigned int ReadCodeInformation(E_FLASH_INFO InfoNum)	 
{
   uint32_t l_temp;
   uint32_t nFlashAddress, nBaseAddress;
  

   nBaseAddress = CheckCodeInformation();
   if ( nBaseAddress==FLASH_READ_ERROR_VALUE || nBaseAddress==0 )
	   return FLASH_READ_ERROR_VALUE;

   SYS_UnlockReg();
   FMC_Open();
   FMC_ENABLE_AP_UPDATE();
	 
   nFlashAddress = nBaseAddress+InfoNum*4;

   l_temp = FMC_Read(nFlashAddress);
	   
  
   FMC_Close();	

   SYS_LockReg();

   return l_temp;
}

uint32_t CheckCodeInformation(void)	 
{
   uint32_t l_temp1;
   uint32_t l_temp2;
   uint32_t BackupData[BackupDataMax];
   //uint32_t *BackupData = (uint32_t *)DataFlashBuffer;
   uint16_t  i;

   SYS_UnlockReg();
    /* Enable FMC ISP functions */
    FMC_Open();
    FMC_ENABLE_AP_UPDATE();
   
   l_temp1 = FMC_Read(CODEINFOBASE+(CHECK_SUM_ADDRESS-1)*4);
	 
   l_temp2 = FMC_Read(TESTPAGEBASE+(CHECK_SUM_ADDRESS-1)*4);
	   	   
   if (  l_temp1==l_temp2 )
   {
      FMC_Close();	
      SYS_LockReg();
      return CODEINFOBASE;
   }

   for(i=0;i<BackupDataMax;i++)
   {	 
   	  BackupData[i] = FMC_Read(CODEINFOBASE+i*4);
   }       
   if ( CalcCrc(BackupData, BackupDataMax)==l_temp1 )
   {
       if ( FMC_Erase(TESTPAGEBASE)==-1 )
       {
           FMC_Close();
           SYS_LockReg();
           return  FLASH_READ_ERROR_VALUE;
     }

     for(i=0;i<BackupDataMax;i++)
     {
	FMC_Write(TESTPAGEBASE+i*4,BackupData[i]);
     } 
     FMC_Write(TESTPAGEBASE+(CHECK_SUM_ADDRESS-1)*4, CalcCrc(BackupData,BackupDataMax));  

       FMC_Close();
       SYS_LockReg();
       return CODEINFOBASE;
   }

   for(i=0;i<BackupDataMax;i++)
   {	 
   	  BackupData[i] = FMC_Read(TESTPAGEBASE+i*4);
   } 
	 
   if ( CalcCrc(BackupData, BackupDataMax)==l_temp2 )
   {
         if ( FMC_Erase(CODEINFOBASE)==-1 )
         {
	    FMC_Close();
                   SYS_LockReg();
	       return  FLASH_READ_ERROR_VALUE;
        }

        for(i=0;i<BackupDataMax;i++)
       {
   	  FMC_Write(CODEINFOBASE+i*4,BackupData[i]);
       } 
  
       FMC_Write(CODEINFOBASE+(CHECK_SUM_ADDRESS-1)*4, CalcCrc(BackupData,BackupDataMax));  

        FMC_Close();	
        SYS_LockReg();
        return TESTPAGEBASE;
   }

   FMC_Close();
   SYS_LockReg();

   return 0;
}
