#pragma once

#include <Prefix_Integer.h>

namespace EARTH {
namespace PT {

/**
* I2C�ʐM�C���^�[�t�F�[�X�N���X�B
*/
class II2C {

public:

  /**
  * I2C�ʐM�̍ő僁�b�Z�[�W���B
  */
  static constexpr uint32 I2C_MSG_MAX = 64;

  /**
  * I2C�����ʁB
  */
  enum Operation {
    READ,		// �ǂݎ��
    WRITE		// ��������
  };

  /**
  * I2C�ʐM���b�Z�[�W�\���́B
  */
  struct Message {
    uint8     addr;	// I2C�A�h���X
    Operation	ope;	// �ǂݎ��or��������
    uint8     len;	// �f�[�^��
    uint8*	  buf;	// �f�[�^�i�o�C�g��j
  };

  /**
  * I2C���b�Z�[�W�̑���M���s���B
  */
  virtual uint32 xfer(Message* msg, uint32 len) = 0;

  /**
  * �R���X�g���N�^�B
  */
  II2C() = default;

  /**
  * �f�X�g���N�^�B
  */
  virtual ~II2C() = default;

private:

  II2C(const II2C&) = delete;
  II2C(II2C&&) = delete;
  II2C& operator =(const II2C&) = delete;
  II2C& operator =(II2C&&) = delete;
};

}
}